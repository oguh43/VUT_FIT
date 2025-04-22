// htab_init.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"

htab_t *htab_init(const size_t n) {
	htab_t *newHtab = malloc(sizeof(htab_t));
	if (newHtab == NULL) {
		return NULL;
	}
	
	newHtab->size = 0;
	newHtab->arr_size = n;
	newHtab->arr_ptr = calloc(sizeof(htab_pair_t**), n); //TODO : REWISE
	if (newHtab->arr_ptr == NULL) {
		free(newHtab);
		return NULL;
	}
	return newHtab;
}
