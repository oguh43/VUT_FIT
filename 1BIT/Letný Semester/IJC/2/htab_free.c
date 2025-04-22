// htab_free.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"

void htab_free(htab_t * t){
	htab_clear(t);
	free(t -> arr_ptr);
	free(t);
}
