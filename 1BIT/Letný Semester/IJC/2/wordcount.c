// wordcount.c
// Řešení IJC-DU2, příklad 2, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include "io.h"
#include "htab.h"
#include <stdio.h>
#include <stdlib.h>

void print(htab_pair_t *pair){
	printf("%s\t%d\n", pair -> key, pair -> value);
}

int main(){
	htab_t *t  =htab_init(5); // TODO : FIGURE OUT A BETTER NUMBER
	if (t == NULL){
		fprintf(stderr, "Error: Could not create hash table!\n");
	}
	char *word = malloc(sizeof(char) * 256); // TODO : FIGURE OUT DIFFERENT MAX LINE LENGTH
	if (word == NULL){
		fprintf(stderr, "Error: Allocation failure!");
		return 1;
	}
	
	while (read_word(word, 256, stdin) != EOF) {
		if (word[0] == '\0'){
			continue;
		}
		htab_lookup_add(t, word) -> value++;
	}
	htab_for_each(t, &print);
	#ifdef STATISTICS
		htab_statistics(t);
	#endif
	free(word);
	htab_free(t);
	return 0;
}
