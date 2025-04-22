// htab__item_t.h
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab.h"
#ifndef IJC_PROJEKT_2_HTAB_ITEM_T_H
#define IJC_PROJEKT_2_HTAB_ITEM_T_H

typedef struct htab_item {
	htab_pair_t pair;
	struct htab_item *next;
} htab_item_t;

#endif //IJC_PROJEKT_2_HTAB_ITEM_T_H
