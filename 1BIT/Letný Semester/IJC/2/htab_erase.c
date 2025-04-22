// htab_erase.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"
#include "htab_resize.h"
#include <stdbool.h>

bool htab_erase(htab_t * t, htab_key_t key){
	size_t hash = htab_hash_function(key);
	
	htab_item_t *items = t->arr_ptr[hash % t->arr_size];
	htab_item_t *prev = NULL;
	while (items != NULL){ // TODO : REPLACE WITH DO WHILE
		if (strcmp(items -> pair.key, key) == 0){
			if (prev == NULL){
				t -> arr_ptr[hash % t -> arr_size] = NULL;
			} else {
				prev -> next = items -> next;
			}
			free((char*) items -> pair.key);
			free(items);
			t -> size = t -> size - 1;
			
			if (t -> size / t -> arr_size < .5){
				t = htab_resize(t, t -> size / 2); // TODO : DYNAMIC RECALCULATION
			}
			return true;
		}
		prev = items;
		items = items -> next;
	}
	return false;
}
