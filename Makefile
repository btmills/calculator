CC = gcc
CFLAGS = -c -Wall
LFLAGS = -Wall
LIBFLAGS=-fPIC -shared -DCACLULATOR_AS_LIB $(LFLAGS)
TESTFLAGS=-DCACLULATOR_AS_LIB -I. -L. $(LFLAGS)
LIBS = -lm

calc: stack.o calculator.c
	$(CC) $(LFLAGS) -o calc calculator.c stack.c $(LIBS)

libcalculator.so: stack.o calculator.c calculator.h
	$(CC) $(LIBFLAGS) -o $@ stack.o calculator.c $(LIBS)

calculator_test: libcalculator.so calculator_test.c
	$(CC) $(TESTFLAGS) -o calculator_test calculator_test.c $(LIBS) -lcalculator

stack_test: stack.o stack_test.c
	$(CC) $(LFLAGS) -o stack_test stack_test.c stack.c $(LIBS)

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c $(LIBS)

clean:
	rm -f *.o calc stack_test libcalculator.so test_calculator

.PHONY: calc
