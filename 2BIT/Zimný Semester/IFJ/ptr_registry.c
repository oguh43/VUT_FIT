/**
 * FIT VUT - IFJ project 2024
 *
 * @file ptr_registry.c
 *
 * @brief Registry of all pointers to be freed at the end of execution
 *
 * @author Hugo Bohácsek (xbohach00)
 */

#include "ptr_registry.h"
#include <stdio.h>
#include "error.h"
// #IALmoment - linked list na uchovávanie
typedef struct PtrNode {
	void *pointer;
	struct PtrNode *next;
} PtrNode;

static PtrNode *head = NULL;

// Plz call len 1x
void ptr_registry_init() {
	head = NULL;
}

// Pridaj ukazateľ do registry
void ptr_registry_add(void *ptr) {
	PtrNode *new_node = (PtrNode *)malloc(sizeof(PtrNode));
	if (!new_node) {
		fprintf(stderr, "Error: Could not allocate memory for registry node\n"); // DEBUG
		error(INTERNAL_ERROR); // QUESTION - toto je interný error????
	}
	new_node -> pointer = ptr;
	new_node -> next = head;
	head = new_node;
}

// Realokuje ukazateľ v registry.
void *ptr_registry_realloc(void *ptr, size_t size){

    void *new_ptr = realloc(ptr, size);
	if (!new_ptr) {  // DEBUG - delete fprintf
		fprintf(stderr, "Error: Memory allocation failed.\n");
		error(INTERNAL_ERROR);
	}

	PtrNode *current = head;
	PtrNode *prev = NULL;

	while (current != NULL) {
		if (current -> pointer == ptr) {
			// Našli sme zhodu
			if (prev) {
				prev -> next = current->next;
			} else {
				head = current -> next;
			}
			free(current -> pointer);
			break;
		}
		prev = current;
		current = current -> next;
	}
	current -> pointer = new_ptr;
	return new_ptr;
}

// Odstráň ukazareľ z registry. Pozor na realokáciu!
void ptr_registry_remove(void *ptr) {
	PtrNode *current = head;
	PtrNode *prev = NULL;

	while (current != NULL) {
		if (current -> pointer == ptr) {
			// Našli sme zhodu
			if (prev) {
				prev -> next = current->next;
			} else {
				head = current -> next;
			}
			if (current -> pointer != NULL){
				free(current -> pointer);
			}
			if (current != NULL){
				free(current);
			}

			return;
		}
		prev = current;
		current = current -> next;
	}
}

// Freene všetky naalokované ukazatele vrátane seba
void ptr_registry_cleanup() {
	PtrNode *current = head;
	while (current != NULL) {
		PtrNode *temp = current;
		current = current -> next;
		if (temp -> pointer != NULL){
			free(temp -> pointer);
		}
		if (temp != NULL){
			free(temp);
		}
	}
	head = NULL;
}
