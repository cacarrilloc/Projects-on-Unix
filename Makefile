CC=gcc
CFLAGS=-g -Wall -std=c99

all:
	${CC} carrilca.adventure.c -o carrilca.adventure ${CFLAGS}

clean:
	rm -f carrilca.adventure
	rm -rf *.rooms.*
	rm -rf carrilca.adventure.dSYM
