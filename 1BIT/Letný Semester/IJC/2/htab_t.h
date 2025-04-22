// htab_t.h
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#include "htab_item_t.h"
#include <stdlib.h>

typedef struct htab{
	size_t size;
	size_t arr_size;
	struct htab_item **arr_ptr;
} htab_t;
