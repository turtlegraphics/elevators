#!/usr/bin/env python

#
# egraphics
#
# v1.1 2008 - updated for cs1graphics version 1.1
# v1.0 2006 - initial version
# Bryan Clair
#
#   Graphics display for elevators simulation
#
# To use, pipe the output of an elevator simulation (e0,e1,e2)
# into stdin for this program.
#
# Requires the cs1graphics v1.1 module by M. Goldwasser, D. Letscher
#

import sys
import time
from cs1graphics import *

# Globals
initialized = False
n_elevators = 1
n_floors = 11
elevators = []
lobbies = []
workspaces = []
doors = []
people = {}

# Appearance Constants
EWIDTH = 100
EGAP = 5
FLOORHEIGHT = 40
LOBBYWIDTH = 100
WSWIDTH = 100
THICK = 3
STATBAR = 30

# Global graphics
walls = Path(Point(EWIDTH,0),Point(0,0),
             Point(0,FLOORHEIGHT),Point(EWIDTH,FLOORHEIGHT))
walls.setBorderWidth(THICK)
door = Path(Point(EWIDTH,0),Point(EWIDTH,FLOORHEIGHT))
door.setBorderWidth(THICK)

time_display = Text("Time: 0")
time_display.setFontSize(14)
warning_display = Text()
warning_display.setFontSize(14)

def anger_color(x):
    return (x,0,0)

# Position calculations
def floor_pos(i):
    "the ceiling height of floor i in pixels"
    return FLOORHEIGHT * (n_floors - i - 1)

def elevator_left(i):
    "the left edge of elevator i in pixels"
    return (EWIDTH + EGAP) * i

def init():
    "Create building and elevators"

    # Calculate canvas dimensions
    WIDTH = n_elevators*(EWIDTH+EGAP) + LOBBYWIDTH + WSWIDTH
    HEIGHT = (n_floors)*FLOORHEIGHT + STATBAR

    # Create building, lobbies, and workspaces
    global building
    building = Canvas(WIDTH,HEIGHT,'grey','Elevators')
    
    lobbyfloor = Path(Point(0,FLOORHEIGHT),Point(LOBBYWIDTH,FLOORHEIGHT))
    lobbyfloor.setBorderColor('white')
    lobbyfloor.setBorderWidth(THICK)

    wsfloor = Path(Point(0,FLOORHEIGHT),Point(WSWIDTH,FLOORHEIGHT))
    wsfloor.setBorderColor('blue')
    wsfloor.setBorderWidth(THICK)
    wswall = Path(Point(0,0),Point(0,FLOORHEIGHT/2))
    wswall.setBorderColor('blue')
    wswall.setBorderWidth(THICK)
    
    for i in range(n_floors):
        lob = Layer()
        lob.add(lobbyfloor)
        lob.moveTo(WIDTH - LOBBYWIDTH - WSWIDTH,floor_pos(i))
        building.add(lob)
        lobbies.append(lob)
        ws = Layer()
        ws.add(wsfloor)
        ws.add(wswall)
        ws.moveTo(WIDTH - WSWIDTH,floor_pos(i))
        building.add(ws)
        workspaces.append(ws)

    # Create elevators
    for i in range(n_elevators):
        e = Layer()
        e.add(walls)
        e.add(door)
        e.moveTo(elevator_left(i),floor_pos(1))
        building.add(e)
        elevators.append(e)

    # Status bar stuff
    time_display.moveTo(0,HEIGHT - STATBAR + 10)
    time_display.setFontColor('skyBlue')
    building.add(time_display)
    warning_display.moveTo(80,HEIGHT - STATBAR + 10)
    warning_display.setFontColor('red')
    building.add(warning_display)

# Person manipulation
MAXNAMEWIDTH = 30
NAMEHEIGHT = 10
NAMESPERSTACK = FLOORHEIGHT/NAMEHEIGHT - 1
num_persons = 0

class Person:
    # location constants
    inE, inL, inW = range(3)
    
    def __init__(self,name,floor):
        global num_persons
        self.image = Text(name)
        self.image.setFontSize(9)
        h = NAMEHEIGHT * (num_persons % NAMESPERSTACK)
        w = ((num_persons/NAMESPERSTACK)*MAXNAMEWIDTH)%(EWIDTH - MAXNAMEWIDTH)
        self.image.moveTo(w,h)
        self.anger = 0
        self.image.setFontColor(anger_color(self.anger))
        num_persons += 1
        self.ltype = Person.inW
        self.lval = floor
        workspaces[floor].add(self.image)

    def __del__(self):
        if self.ltype != Person.inW:
            warn("Teleport : not leaving from workspace")
        self.erase()

    def erase(self):
        if self.ltype == Person.inE:
            elevators[self.lval].remove(self.image)
        elif self.ltype == Person.inL:
            lobbies[self.lval].remove(self.image)
        elif self.ltype == Person.inW:
            workspaces[self.lval].remove(self.image)
        else:
            abort("invalid self.ltype")

    def get_off_at(self,floor):
        if self.ltype != Person.inE:
            warn("Teleport : got off no elevator")
        self.erase()
        self.ltype = Person.inW
        self.lval = floor
        self.anger = 0
        self.image.setFontColor(anger_color(self.anger))
        workspaces[self.lval].add(self.image)

    def go_lobby(self,floor):
        if (self.ltype != Person.inW) or (self.lval != floor):
            warn("Teleport : back to lobby")
        self.erase()
        self.ltype = Person.inL
        self.lval = floor
        lobbies[self.lval].add(self.image)

    def on_elevator(self,e,floor):
        if (self.ltype == Person.inE) and (self.lval == e):
            # already on board
            return
        if (self.ltype != Person.inL) or (self.lval != floor):
            warn("Teleport : onto elevator")
        self.erase()
        self.ltype = Person.inE
        self.lval = e
        elevators[self.lval].add(self.image)
    
    def tickoff(self):
        if (self.ltype == Person.inL):
            self.anger += 2
        if (self.anger > 255):
            self.anger = 255
        self.image.setFontColor(anger_color(self.anger))
    
# Warnings
warning_count = 0
def warn(msg):
    "Display a warning message, keeping count of number of warnings"
    global warning_count
    warning_count += 1
    warning_display.setMessage(`warning_count` + ' ' + msg)
    print "WARNING",msg

# Utility functions
def abort(msg=""):
    sys.stderr.write("Fatal error! ");
    if msg != "":
        sys.stderr.write(msg);
    sys.stderr.write("\n");
    sys.exit()

# Main event loop
while 1:
    try:
        line = raw_input()
    except EOFError:
        sys.exit()

    if line.startswith('!'):
        token = line[1:].split(' ')
    else:
        print line
        continue
    
    if token[0]=="E":
        # Elevator line
        # Syntax: E <enum> <op> <floor> <passengers>
        #         where <op> is "open", "close", or "move"
        enum = int(token[1])
        floor = int(token[3])
        for p in token[4:]:
            people[p].on_elevator(enum,floor)

        if token[2] == "open":
            elevators[enum].remove(door)
        elif token[2] == "close":
            elevators[enum].add(door)
        elif token[2] == "move":
            elevators[enum].moveTo(elevator_left(enum), floor_pos(floor))
        else:
            abort("Bad E operation")
            
    elif token[0]=="P":
        # Person line
        # Syntax: P <name> <op> <floor>
        #         where <op> is "enter", "leave", "on", "move"
        #         <floor> is Person's current floor
        name = token[1]
        if token[2] == "enter":
            people[name] = Person(name,int(token[3]))
        elif token[2] == "leave":
            del people[name]
        elif token[2] == "on":
            people[name].get_off_at(int(token[3]))
        elif token[2] == "move":
            people[name].go_lobby(int(token[3]))
        else:
            abort("Bad P operation")

    elif token[0]=="T":
        # Tick line
        # Syntax: T <time>
        time_display.setMessage("Time: " + token[1])
        for p in people:
            people[p].tickoff()

    elif token[0]=="W":
        # Warning line
        # Syntax: W <warning>
        warn(line[3:])
        
    elif token[0]=="I":
        # Initialize line
        # Syntax: I <floors> <elevators>
        if initialized:
            abort("Multiple initialization lines")
        initialized = True
        n_floors = int(token[1])
        n_elevators = int(token[2])
        init()

    elif token[0]=="F":
        # Finish line
        # Syntax: F <message>
        w = building.getWidth()
        h = FLOORHEIGHT * 3
        fbox = Rectangle(w-4,h-4,Point(w/2,h/2))
        fbox.setFillColor('white')
        fbox.setBorderColor('green')
        fbox.setDepth(-1)
        building.add(fbox)
        results = Text(line[3:] + "\nWarnings: " + `warning_count`)
        results.moveTo(w/2,h/2)
        results.setDepth(-2)
        results.setFontSize(24)
        building.add(results)
        sys.exit()
        
    else:
        abort("bad operation")
