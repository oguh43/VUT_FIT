// htab_resize.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

// Nevedel som to sfunkčniť bez resize. Aj tak by to v realite bolo treba, tak prečo nie...

#include "htab.h"
#include "htab_t.h"
#include <stdio.h>

htab_t* htab_resize(htab_t *t, size_t target) {
	// Allocate memory for the new array
	htab_item_t **new_arr = calloc(target, sizeof(*new_arr));
	if (!new_arr) {
		htab_clear(t);
		htab_free(t);
		return NULL;
	}
	
	// Rehash and move items to the new array
	for (size_t i = 0; i < t->arr_size; ++i) {
		htab_item_t *curr = t->arr_ptr[i];
		while (curr) {
			size_t hash = htab_hash_function(curr->pair.key) % target;
			htab_item_t *next = curr->next;
			curr->next = new_arr[hash];
			new_arr[hash] = curr;
			curr = next;
		}
	}

	free(t->arr_ptr);

	t->arr_ptr = new_arr;
	t->arr_size = target;
	return t;
}
