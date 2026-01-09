/**
 * FIT VUT - IFJ project 2024
 *
 * @file label_list.h
 *
 * @brief Linked list implementation for label handling in generator
 *
 * @author Filip Jenis (xjenisf00)
 */

#ifndef LABEL_LIST_H
#define LABEL_LIST_H

#include "dynamic_string.h"

typedef struct label_list_item {
    dstring_t *value;
    struct label_list_item *prev;
    struct label_list_item *next;
} label_list_item_t;

typedef struct {
    label_list_item_t *first;
    label_list_item_t *last;
    unsigned length;
} label_list_t;

/**
 * Initialization of a list of labels
 * @param list Pointer to the label list struct
 */
void label_list_init(label_list_t *list);

/**
 * Pushes a label to the end of the list
 * @param list Pointer to the label list struct
 * @param val Pointer to the dynamic string label
 */
void label_list_push(label_list_t *list, dstring_t *val);

/**
 * Gets the value of the label on the top of the list
 * @param list Pointer to the label list struct
 * @return Pointer to the label
 */
dstring_t *label_list_top(label_list_t *list);

/**
 * Pops (frees) the label from the top of the list
 * @param list Pointer to the label list struct
 */
void label_list_pop(label_list_t *list);

/**
 * Disposes the whole label list
 * @param list Pointer to the label list structure
 */
void label_list_dispose(label_list_t *list);

#endif //LABEL_LIST_H
