// htab_for_each.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_t.h"
#include "htab_item_t.h"

// for_each: projde všechny záznamy a zavolá na ně funkci f
// Pozor: f nesmí měnit klíč .key ani přidávat/rušit položky
void htab_for_each(const htab_t * t, void (*f)(htab_pair_t *data)){
	for (size_t i = 0; i < t -> arr_size; i++){
		htab_item_t *curr = t -> arr_ptr[i];
		while (curr != NULL){
			f(&curr -> pair);
			curr = curr -> next;
		}
	}
}
