// error.c
// Řešení IJC-DU1, příklad b), 20.3.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "error.h"

void warning(const char *fmt, ...){
    fprintf(stderr, "Warning: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
void error_exit(const char *fmt, ...){
    fprintf(stderr, "Error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}
