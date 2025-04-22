# Makefile
# Řešení IJC-DU1, 20.3.2024
# Autor: Hugo Bohácsek (xbohach00), FIT
# Použit překladač: gcc (GCC) 13.2.1 20230801

CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra
LDFLAGS = -Wall -Wextra
LIBS = -lm
EXECUTABLES = primes primes-i no-comment

ifeq ($(debug), 1)
CFLAGS += -g
LDFLAGS += -g
else
CFLAGS += -O2
endif

ifeq ($(32bit), 1)
CFLAGS += -m32
LDFLAGS += -m32
endif


.PHONY: all run clean primes-i-full

all: primes primes-i no-comment

run: all
	ulimit -s unlimited && ./primes
	ulimit -s unlimited && ./primes-i

clean:
	$(RM) *.o $(EXECUTABLES) *.zip

### A)
primes: primes.o eratosthenes.o error.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

primes.o: primes.c eratosthenes.h bitset.h
	$(CC) $(CFLAGS) -c -o $@ primes.c

eratosthenes.o: eratosthenes.c eratosthenes.h bitset.h
	$(CC) $(CFLAGS) -c -o $@ eratosthenes.c

primes-i: primes-i.o eratosthenes-i.o error.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

primes-i.o: primes.c eratosthenes.h bitset.h
	$(CC) $(CFLAGS) -DUSE_INLINE -c -o $@ primes.c

eratosthenes-i.o: eratosthenes.c eratosthenes.h bitset.h
	$(CC) $(CFLAGS) -DUSE_INLINE -c -o $@ eratosthenes.c

### B)

error.o: error.c error.h
	$(CC) $(CFLAGS) -c -o $@ error.c

no-comment: no-comment.o error.o
	$(CC) $(CFLAGS) -o $@ $^ ${LIBS}

no-comment.o: no-comment.c error.h
	$(CC) $(CFLAGS) -c -o $@ no-comment.c

pack:
	zip xbohach00.zip *.c *.h Makefile
