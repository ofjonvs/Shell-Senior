
CC = gcc -lreadline
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
	 -fstack-protector-all

PROGS = d8sh

.PHONY = all clean

all: $(PROGS)

clean:
	rm -f *.o $(PROGS) a.out

lexer.o: lexer.c parser.tab.h
	$(CC) $(CFLAGS) -c lexer.c

parser.tab.o: parser.tab.c command.h
	$(CC) $(CFLAGS) -c parser.tab.c

executor.o: executor.c command.h executor.h
	$(CC) $(CFLAGS) -c executor.c

d8sh.o: d8sh.c executor.h lexer.h
	$(CC) $(CFLAGS) -c d8sh.c

d8sh: d8sh.o executor.o lexer.o parser.tab.o
	$(CC) -o d8sh d8sh.o lexer.o parser.tab.o executor.o



