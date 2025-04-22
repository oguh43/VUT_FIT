// primes.c
// Řešení IJC-DU1, příklad a), 20.3.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include <stdio.h>
#include <time.h>
#include "eratosthenes.h"

void print_from_back(bitset_t pole, int max);
int main(){
    clock_t start_time = clock();
	
	bitset_create(arr, 666000000);
    eratosthenes(arr);
	
	print_from_back(arr, 10);
	
    fprintf(stderr, "Time=%.3g\n", (double)(clock()-start_time)/CLOCKS_PER_SEC);
    return 0;
}

void print_from_back(bitset_t pole, int max) {
    bitset_index_t last_numbers[max];

    bitset_index_t idx = 0;
    for (bitset_index_t i = bitset_size(pole)-1; i > 0 && idx < 10; i--) {
        if (!bitset_getbit(pole, i)) {
            last_numbers[idx++] = i+1;
        }
    }

    for (int i = max-1; i >= 0; i--) {
        printf("%ld\n", last_numbers[i]);
    }
}
