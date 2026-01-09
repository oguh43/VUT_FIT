/**
 * FIT VUT - IFJ project 2024
 *
 * @file label_list.c
 *
 * @brief Linked list implementation for label handling in generator
 *
 * @author Filip Jenis (xjenisf00)
 */

#include <stdlib.h>
#include <stdio.h>
#include "label_list.h"
#include "error.h"

void label_list_init(label_list_t *list){
    list->first = NULL;
    list->last = NULL;
    list->length = 0;
}


void label_list_push(label_list_t *list, dstring_t *val){
    label_list_item_t *newItem = malloc(sizeof(label_list_item_t));
    if (newItem == NULL){
        fprintf(stderr, "Internal Error: Failed to allocate memory");
        error(INTERNAL_ERROR);
    }
    newItem->value = malloc(sizeof(dstring_t));
    if (newItem->value == NULL){
        fprintf(stderr, "Internal Error: Failed to allocate memory");
        error(INTERNAL_ERROR);
    }
    dynamic_string_init(newItem->value);
    dynamic_string_append(newItem->value, val->string);
    newItem->next = NULL;
    newItem->prev = list->last;
    if (list->last == NULL){
        list->first = newItem;
    }else{
        list->last->next = newItem;
    }
    list->last = newItem;
    list->length++;
}

dstring_t *label_list_top(label_list_t *list){
    if (list->last == NULL){
        return NULL;
    }
    return list->last->value;
}

void label_list_pop(label_list_t *list){
    if (list->length == 0){
        return;
    }
    label_list_item_t *tmpPtr = NULL;
    if (list->length == 1){
        list->first = NULL;
        tmpPtr = NULL;
    }else{
        tmpPtr = list->last->prev;
        tmpPtr->next = NULL;
    }
    dynamic_string_dispose(list->last->value);
    if (list->last != NULL) {
        free(list->last->value);
        free(list->last);
    }
    list->last = tmpPtr;
    list->length--;
}

void label_list_dispose(label_list_t *list){
    if (list->first == NULL){
        return;
    }
    while (list->length != 0) {
        label_list_pop(list);
    }
}