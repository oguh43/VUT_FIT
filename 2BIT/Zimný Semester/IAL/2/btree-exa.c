/*
 * Použití binárních vyhledávacích stromů.
 *
 * S využitím Vámi implementovaného binárního vyhledávacího stromu (soubory ../iter/btree.c a ../rec/btree.c)
 * implementujte triviální funkci letter_count. Všimněte si, že výstupní strom může být značně degradovaný 
 * (až na úroveň lineárního seznamu). Jako typ hodnoty v uzlu stromu využijte 'INTEGER'.
 * 
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Vypočítání frekvence výskytů znaků ve vstupním řetězci.
 * 
 * Funkce inicilializuje strom a následně zjistí počet výskytů znaků a-z (case insensitive), znaku 
 * mezery ' ', a ostatních znaků (ve stromu reprezentováno znakem podtržítka '_'). Výstup je v 
 * uložen ve stromu.
 * 
 * Například pro vstupní řetězec: "abBccc_ 123 *" bude strom po běhu funkce obsahovat:
 * 
 * key | value
 * 'a'     1
 * 'b'     2
 * 'c'     3
 * ' '     2
 * '_'     5
 * 
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce.
*/
void letter_count(bst_node_t **tree, char *input) {
    
    // Inicializujeme strom a vytvoríme pomocné premenné
    bst_init(tree);

    char c;
    bst_node_content_t *result = NULL;

    while (*input) {
        c = *input;

        // Zmeníme veľké písmena na malé
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;
        }

        // Ak je znak písmeno alebo medzera
        if ((c >= 'a' && c <= 'z') || c == ' '){

            // Ak sa znak nachádza v strome, zvýšime jeho hodnotu
            if (bst_search(*tree, c, &result)) {
                (*(int *) (result -> value)) ++;
            } else {
                // Inak vytvoríme nový uzol, s hodnotou 1
                bst_node_content_t new;
                int *tmp = (int *) malloc (sizeof( int));
                *tmp = 1;
                new . value = tmp;
                new . type = INTEGER;
                bst_insert(tree, c, new);
            }
        } else {
            // Ak znak nepatrí, medzi tie, ktoré počítame, zvýšime hodnotu podtržítka
            if (bst_search(*tree, '_', &result)) { // Ak sa znak nachádza v strome, zvýšime jeho hodnotu
                (*(int *) (result -> value)) ++;
            } else { // Inak vytvoríme nový uzol, s hodnotou 1
                bst_node_content_t new;
                int *tmp = (int *) malloc (sizeof( int));
                *tmp = 1;
                new . value = tmp;
                new . type = INTEGER;
                bst_insert(tree, '_', new);
            }
        }
        // Posunieme sa na ďalší znak a vynulujeme result
        input ++;
        result = NULL;
    }
}