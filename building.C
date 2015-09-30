/*
 * building.C
 *
 *  Driver for elevator simulation.
 *  Bryan Clair
 *  2000-2013
 *  Based on elevator project from Jim Plank, U. Tennessee, CS360.
 *
 *  v4.2 9/13
 *       Moving to a single elevators executable, rather than three.
 *       -h command line help.
 *       ElevatorMachinery::getid() function
 *  v4.1 9/08
 *       Fixing g++ compiler warning.
 *  v4.0 10/06
 *       Added -g option for output to graphics display
 *  v3.0 9/06
 *       Redesigned Person class.  take_elevator now an independent
 *       function called by Person threads. Persons now constructed from
 *       strings.  Reads Person definitions on stdin.
 *  v2.9 8/06
 *       Simulation now has a synchronized tick at variable speed,
 *       implemented by the Ticker class.
 *  v2.2 10/04
 *       Added copy constructor and assignment operator for
 *       Person and Identity classes.
 *  v2.1 9/04
 *       Redesign file structure to allow uniform builds of e0,e1,e2,
 *       better command line arguments, versioning.
 *  v2.0 9/03
 *       Complete rewrite in C++ with derived classes.
 *  v1.0 9/01
 *       Direct descendant of Plank's C sources.
 */

#define VERSION "4.2 - 9/30/13"

#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>

#include "building.h"
#include "elevators.h"

using namespace std;

// Forward Declarations
void *el_runner(void *);
void *person_runner(void *);
void *clock_runner(void *);

/*****************************************************
 * Class definitions                                 *
 *****************************************************/
//
// class Building
//    One Building is created, and it creates the Elevators
//
class Building {
public:
  Building(int num_e);  // number of elevators
private:
  Elevator *elist;
  pthread_t *ethreads;
};

//
// class Ticker
//    One Ticker is created to synch the simulation
//
class Ticker {
public:
  Ticker(double speed);    // speed given in seconds
  void once(void);         // wait one tick
  int time(void) const;    // find the simulation time in ticks
  void start(void);        // start the simulation clock (doesn't return)
private:
  volatile int curtime;    // the current simulation tick number
  pthread_mutex_t timelock;
  pthread_cond_t newtick;
  struct timespec one_tick; // time of one tick
};

/*****************************************************
 * Globals                                           *
 *****************************************************/
//
// Global flag for graphics output
//
bool graphics = false;

//
// Global Ticker synchronizes the simulation
//
Ticker *tick;

//
// Global mutex must always be locked before updating the display
//
pthread_mutex_t display_lock;

//
// Globals keep track of trip wait statistics
//
int total_wait;
int total_trips;
pthread_mutex_t stats_lock;

/*****************************************************
 * Top level functions                               *
 *****************************************************/
//
// usage
//   Print a usage error message
//
void usage(char *name, const char *err = NULL)
{
  cerr << "usage: " << name << " [-e elevators] [-s speed] [-g]"
       << endl;
  if (err) cerr << "       " << err << endl;
  exit(1);
}

//
// main
//   Parse arguments.  Create Building and People.
//
main(int argc,char *argv[])
{
  // default values for arguments
  int nelev = 1;
  double speed = .3;

  char opt;
  while ((opt = getopt(argc,argv,"hgs:e:")) != -1)
    switch (opt) {
    case 'g':
      graphics = true;
      break;
    case 's':
      speed = atof(optarg);
      break;
    case 'e':
      nelev = atoi(optarg);
      break;
    case 'h':
    default:
      usage(argv[0]);
    }

  if (optind != argc) usage(argv[0]);
  if (nelev < 1) usage(argv[0],"nelevators must be > 0");
  if (speed <= 0) usage(argv[0],"speed must be > 0");

  // Ensure only one thread writes to display at a time
  pthread_mutex_init(&display_lock,NULL);

  // Initialize statistics counters and lock to protect them
  pthread_mutex_init(&stats_lock,NULL);
  total_wait = 0;
  total_trips = 0;

  // Create a Ticker to count off simulation steps
  tick = new Ticker(speed);

  // Get the display lock to let banner & thread creation errors come first
  pthread_mutex_lock(&display_lock);

  // Display banner
  if (graphics) {
    cout << "!I " << MAXFLOOR+1 << ' ' << nelev << endl;
  }

  cout << "-------------------------------------------\n";
  cout << "Elevators Simulation Version " VERSION << endl;
  cout << "Building with floors 0-" << MAXFLOOR << " and "
       << nelev << " elevators\n";
  cout << "-------------------------------------------\n";
  
  // Create people
  vector<Person *> people;
  string line;
  try {
    while (getline(cin,line)) {
      if ((line[0] != '#') &&
	  (line.find_first_not_of(" \t\n") != string::npos))
	people.push_back(new Person(line));
    }
  } catch (string err) {
    cerr << "Syntax error in input file: " << err << endl;
    exit(1);
  }

  // Create a new building, which creates & runs the elevators
  Building *B = new Building(nelev);

  // Run people
  pthread_t *people_threads = new pthread_t[people.size()];
  for (int i=0; i<people.size(); i++)
    if (pthread_create(people_threads+i,NULL,
		       person_runner,(void *)(people[i])))
      {
	cerr << "Failed to create a person thread.  Try with less people.\n";
	exit(1);
      }

  // Release the display
  pthread_mutex_unlock(&display_lock);

  // Start the Ticker, which starts the simulation
  pthread_t clock_thread;
  pthread_create(&clock_thread,NULL,clock_runner,(void *)tick);

  // Wait for people to exit building
  for (int i=0; i<people.size(); i++) {
    pthread_join(people_threads[i],NULL);
  }


  // Report on timing and exit
  pthread_mutex_lock(&display_lock);  // hold until exit
  cout << "-------------------------------------------\n";
  cout << "Finished in " << tick->time() << " ticks.\n";
  cout << "Average wait ticks per trip: "
       << total_wait/(double)total_trips << endl;
  cout << "-------------------------------------------\n";
  if (graphics) cout << "!F " << "Avg. Wait: "
		     << total_wait/(double)total_trips << endl;
}

//
// el_runner, person_runner, clock_runner
//    As far as I know, this is the way to run a class method on an
//    object in a new thread.
//    Seems like a hack.
//
void *el_runner(void *ev)
{
  ((Elevator *)ev)->run();
}
void *person_runner(void *p)
{
  ((Person *)p)->run();
}
void *clock_runner(void *t)
{
  ((Ticker *)t)->start();
}

/*****************************************************
 * Building class members                            *
 *****************************************************/
Building::Building(int num_e)
{
  elist = new Elevator[num_e];
  ethreads = new pthread_t[num_e];

  for (int i=0; i<num_e; i++)
    if (pthread_create(ethreads+i,NULL,el_runner,(void *)(elist + i))) {
      cerr << "Failed to create an elevator thread."
	   << "  Try with less elevators." << endl;
      exit(errno);
    }
}

/*****************************************************
 * Ticker class members                              *
 *****************************************************/
Ticker::Ticker(double speed)
{
  // Set tick speed
  double secs;
  one_tick.tv_nsec = lround(1000000000*(modf(speed,&secs)));
  one_tick.tv_sec = lround(secs);

  curtime = 0;
  
  pthread_mutex_init(&timelock,NULL);
  pthread_cond_init(&newtick,NULL);
}

//
// Ticker::start
//    Execution thread for the clock.
//    Forever repeats 'wait a tick, wake all sleepers'
//
void Ticker::start(void)
{
  for (;;) {
    pthread_mutex_lock(&display_lock);
    if (graphics)
      cout << "!T " << curtime << endl;
    else 
      cout << "--- tick " << curtime << " ---" << endl;
    pthread_mutex_unlock(&display_lock);
    nanosleep(&one_tick,NULL);
    pthread_mutex_lock(&timelock);
    curtime++;
    pthread_cond_broadcast(&newtick);
    pthread_mutex_unlock(&timelock);
  }
}
    
void Ticker::once(void)
{
  int donetime;
  pthread_mutex_lock(&timelock);
  donetime = curtime + 1;
  while (donetime != curtime)
    pthread_cond_wait(&newtick,&timelock);
  pthread_mutex_unlock(&timelock);
}

int Ticker::time(void) const
{
  return curtime;
}

/*****************************************************
 * ElevatorMachinery class members                   *
 *****************************************************/
// initialize static member that handles id assignment
int ElevatorMachinery::next_unused_id = 0;

//
// ElevatorMachinery constructor
//
ElevatorMachinery::ElevatorMachinery()
{
  id = next_unused_id++;  // generate a new id for this elevator
  floor = 1;
  door_status = false;
}

int ElevatorMachinery::getid() const {return id;}
  
//
// Elevator display functions
//
void ElevatorMachinery::display()
{
  int n;

  cout << "[Elevator " << id << "]";
  cout << " (carrying ";
  if ((n=display_passengers()) > 0) 
    cout << ")";
  else
    cout << "nobody)";
}

void ElevatorMachinery::message(const char *s)
  // Generate a message for this elevator on the screen
  // This is the approved method for Elevator objects to output information
{
  pthread_mutex_lock(&display_lock);
  display();
  cout << ": " << s << endl;
  pthread_mutex_unlock(&display_lock);
}

void ElevatorMachinery::message(const char *s, const int i)
  // Variant which also prints a numerical argument.
{
  pthread_mutex_lock(&display_lock);
  display();
  cout << ": " << s << " " << i << endl;
  pthread_mutex_unlock(&display_lock);
}

void ElevatorMachinery::gmessage(const char *s)
  // Print an action command to the graphics package
{
  pthread_mutex_lock(&display_lock);
  cout << "!E " << id << ' ' << s << ' ' << floor;
  display_passengers();
  cout << endl;
  pthread_mutex_unlock(&display_lock);
}

void ElevatorMachinery::warning(const char *s)
  // Generate a warning message for this elevator on the screen
  // This is a private member function, and is called by the movement
  // functions when physically impossible things are tried.
{
  pthread_mutex_lock(&display_lock);
  if (graphics) cout << "!W E" << id << ":" << s << endl;
  else cerr << "WARNING: [ELEVATOR " << id << "] " << s << endl;
  pthread_mutex_unlock(&display_lock);
}

//
// Elevator status functions
//
int ElevatorMachinery::onfloor() const {return floor;}
bool ElevatorMachinery::door_is_open() const {return door_status;}

//
// Elevator action functions
//
void ElevatorMachinery::move_to_floor(int dest) {
  int dir;

  // Make sure door is closed
  if (door_is_open()) {
    warning("move_to_floor called with door open");
    return;
  }

  // Check for valid destination
  if (dest > MAXFLOOR) {
    warning("move_to_floor called with destination too large");
    dest = MAXFLOOR;
  }
  if (dest < 0) {
    warning("move_to_floor called with negative destination");
    dest = 0;
  }

  dir = (floor > dest)? -1 : 1;

  // Move the elevator
  while (floor != dest) {
    tick->once();
    floor += dir;
    graphics ? gmessage("move") : message("moved to floor",floor);
  }
}

void ElevatorMachinery::move_up()
{
  if (door_is_open()) {
    warning("move_up called with door open");
    return;
  }
  if (floor == MAXFLOOR)
    warning("move_up can't move any higher!");
  else {
    tick->once();
    floor++;
    graphics ? gmessage("move") : message("moved to floor",floor);
  }
}

void ElevatorMachinery::move_down()
{
  if (door_is_open()) {
    warning("move_down called with door open");
    return;
  }
  if (floor == 0)
    warning("move_down can't move any lower!");
  else {
    tick->once();
    floor--;
    graphics ? gmessage("move") : message("moved to floor",floor);
  }
}

void ElevatorMachinery::open_door()
{
  tick->once();
  if (door_is_open()) {
    warning("open_door called with door already open");
  } else {
    graphics ? gmessage("open") : message("door is open");
    door_status = true;
  }
}

void ElevatorMachinery::close_door()
{
  tick->once();
  if (!door_is_open()) {
    warning("close_door called with door already closed");
    return;
  } else {
    graphics ? gmessage("close") : message("door is closed");
    door_status = false;
  }
}

/*****************************************************
 * Person class members                              *
 *****************************************************/
//
// Construct Person from a string
//   String format: Name starttime floor1 worktime1 ... floorN worktimeN
//
Person::Person(string line)
{
  istringstream linein(line);

  linein >> name;
  if (!linein) throw(string("bad name"));
  linein >> entrytime;
  if (!linein) throw("bad entrytime for " + name);

  floor = 1;

  int nfloor,ntime;
  linein >> nfloor;
  do {
    linein >> ntime;
    if (!linein) throw("bad work floor or time for " + name);
    work_floors.push_back(nfloor);
    work_times.push_back(ntime);
  } while (linein >> nfloor);
}

//
// Person display functions
//
void Person::display() const
{
  if (graphics) cout << " " << name;
  else cout << "[" << name << "]";
}

void Person::message(const char *s) const
{
  pthread_mutex_lock(&display_lock);
  display();
  cout << ": " << s << endl;
  pthread_mutex_unlock(&display_lock);
}

void Person::message(const char *s, int i) const
{
  pthread_mutex_lock(&display_lock);
  display();
  cout << ": " << s << " " << i << endl;
  pthread_mutex_unlock(&display_lock);
}

void Person::gmessage(const char *s) const
{
  pthread_mutex_lock(&display_lock);
  cout << "!P";
  display();
  cout << ' ' << s << ' ' << floor << endl;
  pthread_mutex_unlock(&display_lock);
}

void Person::warning(const char *s) const
{
  pthread_mutex_lock(&display_lock);
  if (graphics) {
    cout << "!W " << name << ' ' << s << endl;
  } else {
    cerr << "WARNING: ";
    cerr << "[" << name << "]";
    cerr << ": " << s << endl;
  }
  pthread_mutex_unlock(&display_lock);
}

//
// Person::run
//
//    Called only once, when a new Person is created
//    Return value is average wait time per trip.
//    Wait time for a trip is ticks in excess of the distance to
//    travel plus one for closing doors when boarding and one
//    for opening doors when getting off.
//
double Person::run()
{
  int i;
  int my_wait_time = 0,trip_start,trip_wait;

  while (entrytime--) tick->once();

  graphics ? gmessage("enter") : message("entered building on floor",floor);

  for (i=0; i < work_floors.size(); i++) {
    graphics ? gmessage("move") : message("moving to floor",work_floors[i]);

    trip_start = tick->time();

    take_elevator(this,floor,work_floors[i]);

    trip_wait = (tick->time() - trip_start)
      - 2 // for doors
      - abs(floor - work_floors[i]); // for distance

    if (trip_wait < 0)
      warning("trip took too little time");

    my_wait_time += trip_wait;

    floor = work_floors[i];
    if (graphics) gmessage("on");
    else {
      message("on floor",floor);
      message("working for",work_times[i]);
    }
    while (work_times[i]-- > 0) tick->once();
  }

  // Update global stats
  pthread_mutex_lock(&stats_lock);
  total_wait += my_wait_time;
  total_trips += work_floors.size();
  pthread_mutex_unlock(&stats_lock);

  graphics ? gmessage("leave") : message("leaving the building");
}
