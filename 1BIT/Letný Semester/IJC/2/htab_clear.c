// htab_clear.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"

void _recursive_destructor(struct htab_item *i) {
	if(i == NULL) {
		return;
	}
	
	_recursive_destructor(i->next);
	free((char*) i->pair.key);
	free(i);
}
void htab_clear(htab_t * t){
	
	for (size_t i = 0; i < t -> arr_size; i++){
		struct htab_item *current_item = t->arr_ptr[i];
		_recursive_destructor(current_item); //https://www.geeksforgeeks.org/nested-functions-c/
		t->arr_ptr[i] = NULL;
	}
	t -> size = 0;
}
