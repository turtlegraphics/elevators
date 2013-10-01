//
// elevators.h
//

#ifndef ELEVATORS_H
#define ELEVATORS_H

#include "building.h"

//
//  One Elevator object will be created for each elevator
//  in the simulation.
//
class Elevator : public ElevatorMachinery {
 public:
  Elevator();

  //
  // run
  //   will be called at the beginning of the simulation, to put the
  //   Elevator into operation.  run should pick up and deliver Persons,
  //   coordinating with other Elevators for efficient service.
  //   run should never return.
  //   
  void run();

  //
  // display_passengers
  //   should call display() for each Person on the elevator
  //   and return the number of Person displayed
  //
  int display_passengers();

 private:

};

//
//  take_elevator
//
//    A Person (who) calls this function to take an elevator from their
//    current floor (origin) to a different floor (destination).
//
void take_elevator(const Person *who, int origin, int destination);

#endif

