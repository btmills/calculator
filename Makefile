CC = gcc
CFLAGS = -c -Wall
LFLAGS = -Wall

calc: stack.o calculator.c
	$(CC) $(LFLAGS) -o calc calculator.c stack.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c

clean:
	rm -f *.o calc
