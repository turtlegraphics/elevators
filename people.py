#!/usr/bin/env python
"""
people

2006-2015
Bryan Clair

Generate person records for the elevators simulation.
"""

usage = """
usage: people [-p people] [-t trips] [-d delaymax]
"""

import random
import itertools

MAXFLOOR = 10

fnames = [
  "Phil",     "Pat",      "Peyton",  "Chamique",
  "Jamal",    "Tamika",   "Terry",   "Teresa",
  "Peerless", "Semeka",   "Al",      "Kristin",
  "Tori",     "Misty",    "Marcus",  "Kellie",
  "Leonard",  "LaShonda", "Trey",    "Byrnae",
  "Mercedes", "Kyra",     "Jonathan","Niya"
]

lnames = [
  "Fulmer",   "Summitt",  "Manning", "Holdsclaw",
  "Lewis",    "Catchings","Fair",    "Geter",  
  "Price",    "Randall",  "Wilson",  "Clement", 
  "Noel",     "Greene",   "Nash",    "Jolly",
  "Little",   "Stevens",  "Teague",  "Laxton",
  "Hamilton", "Elzy",     "Brown",   "Butts"
]


def skewproduct(a,b):
  """
  Similar to itertools.product, forms the product of two iterators a,b,
  but in the order:
  (a[0],b[0]) , (a[1],b[1]) , ... , (a[n],b[n]) , 
  (a[0],b[1]) , (a[1],b[2]) , ... , (a[n],b[0]) ,
  ...
  (a[0],b[n]) , (a[1],b[0]) , ... , (a[n],b[n-1])

  Example:
  skewproduct("ABC","123") yields A1, B2, C3, A2, B3, C1, A3, B1, C2
  """
  l = len(a)
  assert(l == len(b))
  for offset in range(l):
    for f in range(l):
      yield (a[f],b[(f+offset) % l])

class Person:
  """
  A person, with things to do in the building.
  """

  """
  Name generator using:
  1) FirstName, until those run out, then
  2) FirstName_LastName, making all possible combos, then
  3) Person## for numbers 0,1,2,...
  """
  fullnames = itertools.chain(
    fnames,
    ('_'.join(x) for x in skewproduct(fnames,lnames)),
    ('Person'+str(x) for x in itertools.count())
    )

  def __init__(self,trips,delaymax):
    self.name = Person.fullnames.next()

    self.start = random.randint(1,delaymax)
    self.tasks = []

    floor = 1
    for i in range(trips):
      floor = random.choice([x for x in range(MAXFLOOR+1) if x != floor])
      delay = random.randint(1,delaymax)
      self.tasks.append((floor,delay))

  def __str__(self):
    out = self.name
    out += ' '
    out += str(self.start)
    for (floor, delay) in self.tasks:
      out += ' ' + str(floor) + ' ' + str(delay)
    return out

if __name__=='__main__':
  import argparse

  def positive_int(s):
    if int(s) <= 0:
      raise argparse.ArgumentTypeError("must be positive")
    return int(s)

  parser = argparse.ArgumentParser()
  parser.add_argument("-p","--people",type=int,default=5,
                      help="Number of people to create.")
  parser.add_argument("-t","--trips",type=int,default=5,
                      help="Number of trips each person will make")
  parser.add_argument("-d","--delaymax",type=int,default=10,
                      help="Maximum delay on a given floor.")
  args = parser.parse_args()

  for i in range(args.people):
    print Person(args.trips,args.delaymax)
