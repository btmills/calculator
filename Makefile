CC = gcc
CFLAGS = -Wall
LFLAGS = -Wall
LIBFLAGS=-fPIC -shared -DCALCULATOR_AS_LIB $(LFLAGS)
TESTFLAGS=-DCALCULATOR_AS_LIB -I. -L. $(LFLAGS)
LIBS = -lm
Q = @

ifneq ($(findstring $(MAKECMDGOALS),_test),)
CFLAGS += $(LFLAGS)
endif
ifneq ($(findstring $(MAKECMDGOALS),.so),)
CFLAGS += $(LFLAGS)
endif

all: calc

calc: stack.o calculator.o
	$(Q)$(CC) $(CFLAGS) -o calc calculator.o stack.c $(LIBS)

libcalculator.so: stack.lib.o calculator.lib.o
	$(Q)$(CC) $(LIBFLAGS) -o $@ stack.lib.o calculator.lib.o $(LIBS)

calculator_test: libcalculator.so calculator_test.c
	$(Q)$(CC) $(TESTFLAGS) -o calculator_test calculator_test.c $(LIBS) -lcalculator

stack_test: stack.o stack_test.c
	$(Q)$(CC) $(LFLAGS) -o stack_test stack_test.c stack.c $(LIBS)

stack.o: stack.c stack.h
	$(Q)$(CC) $(CFLAGS) -c stack.c -o $@ $(LIBS)

calculator.o: calculator.c calculator.h
	$(Q)$(CC) $(CFLAGS) -c calculator.c -o $@ $(LIBS)

stack.lib.o: stack.c stack.h
	$(Q)$(CC) $(LIBFLAGS) -c stack.c -o $@ $(LIBS)

calculator.lib.o: calculator.c calculator.h
	$(Q)$(CC) $(LIBFLAGS) -c calculator.c -o $@ $(LIBS)

clean:
	$(Q)rm -f *.o calc stack_test libcalculator.so calculator_test

.PHONY: calc clean
