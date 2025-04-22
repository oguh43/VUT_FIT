// eratostenes.c
// Řešení IJC-DU1, příklad a), 20.3.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "eratosthenes.h"
#include <math.h>

#ifdef USE_INLINE
extern void bitset_free(bitset_t jmeno_pole);
extern bitset_index_t bitset_size(bitset_t jmeno_pole);
extern void bitset_setbit(bitset_t jmeno_pole, bitset_index_t index, int bool_vyraz);
extern int bitset_getbit(bitset_t jmeno_pole, bitset_index_t index);
#endif

void eratosthenes(bitset_t arr){
    unsigned int bit;
    for (bitset_index_t index = 1; index < sqrt(bitset_size(arr)); index++){
        bit = bitset_getbit(arr, index);
        bitset_setbit(arr, 0, 1); // hacky fix
        if (!bit){
            for (bitset_index_t idx = (index * 2) + 2; idx < bitset_size(arr) + 1; idx += index + 1){
                bitset_setbit(arr, idx - 1, 1);
            }
        }
    }
}
















