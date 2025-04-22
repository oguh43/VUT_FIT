// bitset.h
// Řešení IJC-DU1, příklad a), 20.3.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#ifndef IJC_PROJEKT_1_BITSET_H
#define IJC_PROJEKT_1_BITSET_H

#include "error.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

typedef unsigned long int *bitset_t;
typedef unsigned long int bitset_index_t;

#define bitset_calc_size(velikost) velikost / (CHAR_BIT * sizeof(unsigned long int)) + 1 + (velikost % (CHAR_BIT * sizeof(unsigned long int)) != 0)


#define bitset_create(jmeno_pole, velikost)\
    static_assert(velikost > 0, "Velikost <= 0 !");\
    bitset_index_t jmeno_pole[bitset_calc_size(velikost)] = {velikost};

#define bitset_alloc(jmeno_pole, velikost)\
    assert( velikost > 0 );\
    bitset_t jmeno_pole = calloc(bitset_calc_size(velikost), sizeof(unsigned long int));\
    if (jmeno_pole == NULL){error_exit("bitset_alloc: Chyba alokace paměti");}\
    jmeno_pole[0] = velikost;


#ifndef USE_INLINE
#define bitset_free(jmeno_pole) free(jmeno_pole) //TODO: check if NULL!!!
#define bitset_size(jmeno_pole) jmeno_pole[0]

#define bitset_setbit(jmeno_pole, index, bool_vyraz) do{\
    bitset_index_t where = index + (CHAR_BIT * sizeof(unsigned long int));\
    if (where >= bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int)){\
        error_exit("bitset_setbit: Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int));\
    }else{\
        if (bool_vyraz) {\
            jmeno_pole[where / (CHAR_BIT * sizeof(unsigned long int))] |= (1L << (where % (CHAR_BIT * sizeof(unsigned long int))));\
        }else{\
            jmeno_pole[where / (CHAR_BIT * sizeof(unsigned long int))] &= ~(1L << (where % (CHAR_BIT * sizeof(unsigned long int))));\
        }\
    }\
} while(0);

#define bitset_getbit(jmeno_pole, index) ( \
    (index + sizeof(unsigned long) * CHAR_BIT >= bitset_size(jmeno_pole) + (CHAR_BIT * sizeof(unsigned long int))) ? \
    error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bitset_size(jmeno_pole) + (CHAR_BIT * sizeof(unsigned long int))),1 :\
    (int) (1 & (jmeno_pole[((index + (CHAR_BIT * sizeof(unsigned long int))) / (CHAR_BIT * sizeof(unsigned long int)))] >> (( index + (CHAR_BIT * sizeof(unsigned long int))) % (CHAR_BIT * sizeof(unsigned long int))))))

#define bitset_fill(jmeno_pole, bool_vyraz) for(bitset_index_t index=1; index < bitset_size(jmeno_pole); index++){bitset_setbit(jmeno_pole, index, bool_vyraz)};

#else //USE_INLINE else

inline void bitset_free(bitset_t jmeno_pole){
    free(jmeno_pole);
}
inline bitset_index_t bitset_size(bitset_t jmeno_pole){
    return jmeno_pole[0];
}
inline void bitset_setbit(bitset_t jmeno_pole, bitset_index_t index, int bool_vyraz){
    bitset_index_t where = index + (CHAR_BIT * sizeof(unsigned long int));
    if (where >= bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int)){
        error_exit("bitset_setbit: Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int));
    }else{
        if (bool_vyraz) {
            jmeno_pole[where / (CHAR_BIT * sizeof(unsigned long int))] |= (1L << (where % (CHAR_BIT * sizeof(unsigned long int))));
        }else{
            jmeno_pole[where / (CHAR_BIT * sizeof(unsigned long int))] &= ~(1L << (where % (CHAR_BIT * sizeof(unsigned long int))));
        }
    }
    return;
}
inline int bitset_getbit(bitset_t jmeno_pole, bitset_index_t index){
    // Z tohoto je robené to makro... Insanity :(((((((((
    bitset_index_t where = index + (CHAR_BIT * sizeof(unsigned long int));
    if (where >= bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int)){
        error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bitset_size(jmeno_pole) + CHAR_BIT * sizeof(unsigned long int));
    }
    return 1 & (jmeno_pole[where / (CHAR_BIT * sizeof(unsigned long int))] >> (where % (CHAR_BIT * sizeof(unsigned long int))));
}
inline void bitset_fill(bitset_t jmeno_pole, int bool_vyraz){
    for (bitset_index_t index=1; index < bitset_size(jmeno_pole); index++){
        bitset_setbit(jmeno_pole, index, bool_vyraz);
    }
    return;
}

#endif //USE_INLINE endif

#endif //IJC_PROJEKT_1_BITSET_H
