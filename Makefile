CC = gcc
CFLAGS = -c -Wall
LFLAGS = -Wall
LIBS = -lm

calc: stack.o calculator.c
	$(CC) $(LFLAGS) -o calc calculator.c stack.c $(LIBS)

stack_test: stack.o stack_test.c
	$(CC) $(LFLAGS) -o stack_test stack_test.c stack.c $(LIBS)

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c $(LIBS)

clean:
	rm -f *.o stack_test

.PHONY: calc
