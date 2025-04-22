// htab_lookup_add.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"
#include "htab_resize.h"
#include <stdio.h>

htab_pair_t * htab_lookup_add(htab_t * t, htab_key_t key){
	size_t hash = htab_hash_function(key);
	htab_item_t *items = t -> arr_ptr[hash % t -> arr_size];
	htab_item_t *prev = NULL;
	
	while(items != NULL) {
		if(strcmp(items -> pair.key, key) == 0) {
			return &items -> pair;
		}
		prev = items;
		items = items -> next;
	}
	
	htab_item_t *new = malloc(sizeof(htab_item_t));
	if (new == NULL){
		return NULL;
	}
	
	char *key_copy = malloc(sizeof(char) * (strlen(key) + 1));
	if (key_copy == NULL){
		free(new);
		return NULL;
	}
	strcpy(key_copy, key);
	
	new -> pair.key = key_copy;
	new -> pair.value = 0;
	new -> next = NULL;
	t -> size = t -> size + 1;
	
	if (items != NULL){
		items -> next = new;
	} else if (prev != NULL){
		prev -> next = new;
	} else {
		t -> arr_ptr[hash % t -> arr_size] = new;
	}
	
	if (t -> size / t -> arr_size > 3){ // TODO : DYNAMICALLY CALCULATE
		t = htab_resize(t, t -> size * 2);
	}
	return &new -> pair;
}
