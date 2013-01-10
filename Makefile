CC = gcc
CFLAGS = -Wall
LDFLAGS = $(CFLAGS)

all: calc
calc: calculator.c stack.c stack.h
	   $(CC) $(CFLAGS) -o calc calculator.c stack.c

clean:
	   rm -f *.o calc
