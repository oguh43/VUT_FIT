// htab_statistics.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "htab_t.h"
#include "htab_item_t.h"
#include <stdio.h>

void htab_statistics(const htab_t *table) {
	size_t min_bucket_size = (size_t)-1;
	size_t max_bucket_size = 0;
	size_t non_empty_buckets = 0;
	
	for (size_t index = 0; index < table->arr_size; ++index) {
		size_t count = 0;
		non_empty_buckets += table->arr_ptr[index] != NULL;
		for (htab_item_t *node = table->arr_ptr[index]; node; node = node->next)
			++count;
		if (count > max_bucket_size)
			max_bucket_size = count;
		if (count < min_bucket_size)
			min_bucket_size = count;
	}
	
	if (table->size == 0) {
		min_bucket_size = 0;
		non_empty_buckets = 1;
	}
	
	fprintf(stderr, "Bucket count:  %zu\nMax items in bucket: %zu\nMin items in bucket: %zu\nAverage items in bucket: %lf\n", table->arr_size, max_bucket_size, min_bucket_size, (double)table->size / non_empty_buckets);
}
