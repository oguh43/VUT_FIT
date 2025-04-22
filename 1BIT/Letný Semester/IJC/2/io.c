// io.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "io.h"
#include <ctype.h>

int read_word(char *s, int max, FILE *f) {
	int c;
	while (isspace(c = fgetc(f)));
	if (c == EOF) {
		return EOF;
	}
	int i = 0;
	do {
		s[i++] = c;
	} while (i < max - 1 && (c = fgetc(f)) != EOF && !isspace(c));
	s[i] = 0;
	if (!isspace(c)) {
		while (!isspace(c = fgetc(f)) && c != EOF);
	}
	return i;
}
