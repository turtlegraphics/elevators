/*
 * building.h
 * Bryan Clair
 */
#ifndef _BUILDING_H_
#define _BUILDING_H_

#include <string>
#include <vector>

//
//  The building has floors numbered 0, 1, 2, ... , MAXFLOOR
//
#define MAXFLOOR 10

//
// class ElevatorMachinery
//
//     Implements the physical model of an elevator.
//     The algorithm and coordination with passengers are expected
//     to be implemented in a derived class Elevator.
//
class ElevatorMachinery {
 public:
  ElevatorMachinery();

  int getid() const;                   // unique ID for this elevator

  //
  // Elevator display functions
  //
  void message(const char *s);        // display a message for this elevator
  void message(const char *s, const int i);  // variant also displays an int

  virtual int display_passengers() = 0;  // returns number of passengers
  // The Elevator class must implement display_passengers since
  // ElevatorMachinery is not responsible for maintaining a list of passengers

  //
  // Elevator status functions
  //
  int onfloor() const;                 // elevator's current floor
  bool door_is_open() const;           // true if door is open

  //
  // Elevator action functions
  //
  void move_to_floor(int floor);
  void move_up();                      // up one floor
  void move_down();                    // down one floor
  void open_door();
  void close_door();

 private:
  static int next_unused_id;
  int id;
  int floor;
  bool door_status;  // true if door is currently open
  void display();                     // display the elevator
  void warning(const char *s);
  void gmessage(const char *s);
  // display a warning message if machinery used improperly
};

//
// class Person
//
//     Implements a person in the building.
//     The person has work to do on various floors, and alternates
//     between "working" and taking an elevator to a new floor.
//
//     The Person class decides how long to "work" and decides which
//     floors to visit.  In addition, the Person class keeps track
//     of time spent waiting for elevators, and displays the Person's
//     progress through the building.
//
//     The Person class does NOT implement the act of taking an elevator,
//     but (repeatedly) calls take_elevator.
//
class Person {
 public:
  Person();
  Person(std::string);
  //  ~Person();
  //  Person(const Person&);
  //  Person& operator=(const Person&);

  //
  // Person display functions
  //
  void display() const;                 // display this person
  void message(const char *s) const;    // display a message for this person
  void message(const char *s, const int i) const;
                                        // variant also displays an int

 private:
  int floor;
  std::string name;
  int entrytime;
  std::vector<int> work_floors;
  std::vector<int> work_times;
  void gmessage(const char *s) const;
  void warning(const char *s) const;
  double run(void);
  friend void *person_runner(void *);
};

#endif











