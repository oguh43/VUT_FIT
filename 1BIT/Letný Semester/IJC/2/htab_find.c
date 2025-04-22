// htab_find.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"

htab_pair_t * htab_find(const htab_t * t, htab_key_t key){
	size_t hash = htab_hash_function(key);
	htab_item_t *items = t -> arr_ptr[hash % t -> arr_size];
	while (items != NULL){
		if (strcmp(items -> pair.key, key) == 0){
			return &items -> pair;
		}
		items = items -> next;
	}
	return NULL;
}
