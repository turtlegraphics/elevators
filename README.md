elevators
=========

Multithreaded elevator simulation for Saint Louis University CS 3500 Operating Systems.

basic operation
---------------
Build with `make elevators`

Usage:
 `elevators [-e nelevators] [-s speed] [-g]`

This program simulates a building with elevators.
The building has 11 floors numbered from 0 to 10.
People enter the building on floor 1 at various times. They take elevators to various
floors, spend time working on those floors, and eventually exit the building on floor 1.

options
-------
* `-e nelevators`: Set the number of elevators in the simulation
* `-s speed`: The simulation proceeds with discrete time ticks, and each tick takes `speed` seconds.
* `-g`: Use output formatted for graphics, see below.

Each event that occurs in the simulation will be indicated by one line of output.

person records
--------------
The people and their behavior are described by a series of single line text records
fed to the simulation on stdin.

Each person’s record is a line:
 `name starttime floor1 worktime1 ... floorN worktimeN`

The person will enter the first floor at `starttime`, proceed to `floor1`,
work for `worktime1`, then proceed to `floor2` and so on until `floorN`, where they work
for `worktimeN` and then take an elevator to the first floor to exit.

### .eld files
There are a few `.eld` files in the distribution for testing.  You might run the program with:

 `elevators < oneguy.eld`

to direct the `oneguy.eld` file to elevators standard input.

### people generator
There is a utility program `people.py` that will generate random person records.

Usage:
 `people.py [-p people] [-t trips] [-d delaymax]`

For example,

`people.py -p3 -t8 -d10 | elevators`

will generate 3 people, each of which takes 8 trips and works for up to 10 ticks between trips, then pipe the output into elevators.

graphics
--------
The file `egraphics.py` is a graphical front end to the simulation.  The elevators
simulation, when run with the `-g` option, produces output that can be fed to the input
of `egraphics.py` with a pipe.  For example,

`elevators -e3 -s0.5 -g < tenpeople.eld | egraphics.py`

assignment
----------
This is an assignment for CS 3500 at Saint Louis University.
As such, the simulation does not actually work yet.

The assignment is available at:
http://mathcs.slu.edu/~clair/os
