#
# Makefile for elevator simulations
#

# If you create additional header and/or source files, add them here:
eheaders = elevators.h building.h
esources = elevators.C
elibs = -lpthread

#
# Rule for making elevators
#
#    CXX is a built in variable that evaluates to the default C++ compiler.
#
#    CXXFLAGS evaluates to the default flags for compilation, usually none.
#    Build with "make CXXFLAGS=-g", for example, to generate debugger hooks.
#
elevators: $(esources) $(eheaders) building.o
	$(CXX) $(CXXFLAGS) $(esources) building.o -o $@ $(elibs)

building.o: building.h building.C elevators.h
	$(CXX) $(CXXFLAGS) -c building.C -o $@

clean:
	rm -f building.o elevators
