//
//    elevators.C
//
//    Function stubs for the missing parts of elevators.
//
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include "building.h"
#include "elevators.h"

//
// Elevator constructor
//   Called once for each elevator before the thread is created.
//
Elevator::Elevator()
{ 
  // This is the place to perform any one-time initializations of
  // per-elevator data.
}

//
// Elevator::display_passengers()
//
//  Call display() for each Person on the elevator.
//  Return the number of riders.
//
//  Beware: calling message() from this function will hang the simulation.
//
int Elevator::display_passengers()
{
  return 0;
}

//
// Elevator::run()
//
//   Main thread for an elevator.
//   Will be called at the beginning of the simulation, to put the
//   Elevator into operation.  run() should pick up and deliver Persons,
//   coordinating with other Elevators for efficient service.
//   run should never return.
//   
void Elevator::run()
{
  message("running");
  // Pick up and drop off passengers.
}

//
//  take_elevator
//
//    A Person (who) calls this function to take an elevator from their
//    current floor (origin) to a different floor (destination).
//
void take_elevator(const Person *who, int origin, int destination)
{
  // Should not return until the person has taken an elevator
  // to their destination floor.
}

