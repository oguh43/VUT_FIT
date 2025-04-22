# Makefile
# Řešení IJC-DU2, příklad 2, 18.4.2024
# Autor: Hugo Bohácsek (xbohach00), FIT
CC=gcc
CFLAGS=-g -fPIC -Wextra -Wall -pedantic -std=c11
LIBS=-L. -lhtab

SOURCES := $(wildcard htab*.c)
HEADERS := $(wildcard *.h)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

all: tail wordcount-dynamic wordcount

run: all

tail: tail.o
	$(CC) $(CFLAGS) $^ -o $@

wordcount: libhtab.a wordcount.o io.o
	$(CC) $(CFLAGS) -static wordcount.o io.o -o $@ $(LIBS)

wordcount-dynamic: wordcount.o io.o libhtab.so
	$(CC) $(CFLAGS) wordcount.o io.o -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

libhtab.a: $(OBJECTS)
	ar crs $@ $^

libhtab.so: $(OBJECTS)
	$(CC) -shared $^ -o $@

clean:
	$(RM) *.o *.a *.so tail wordcount wordcount-dynamic

submit: clean
	zip xbohach00.zip $(SOURCES) $(HEADERS) Makefile

