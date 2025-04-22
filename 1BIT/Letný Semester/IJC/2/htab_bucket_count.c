// htab_bucket_count.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"

size_t htab_bucket_count(const htab_t * t){
	return t -> arr_size;
}
