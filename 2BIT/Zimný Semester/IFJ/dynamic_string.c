/**
 * FIT VUT - IFJ project 2024
 *
 * @file dynamic_string.c
 *
 * @brief Dynamic string implementation for code generator
 *
 * @author Filip Jenis (xjenisf00)
 */

#include "dynamic_string.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>

void dynamic_string_init(dstring_t *str){
    str->string = malloc(DYNAMIC_STRING_INIT_SIZE*sizeof(char));
    if (str->string == NULL){
        fprintf(stderr, "Internal Error: Failed to allocate memory");
        error(INTERNAL_ERROR);
    }
    str->string[0] = '\0';
    str->curr_len = 0;
    str->allocated = DYNAMIC_STRING_INIT_SIZE;
}

void dynamic_string_dispose(dstring_t *str){
    if (str->string != NULL) {
        free(str->string);
    }
    str->curr_len = 0;
    str->allocated = 0;
}

void dynamic_string_append(dstring_t *str, const char *append){
    if (str->curr_len + strlen(append) + 1 >= str->allocated){
        unsigned reallocSize = str->allocated;
        while (reallocSize < str->curr_len + strlen(append) + 1){
            reallocSize *= 2;
        }
        str->string = realloc(str->string, reallocSize*sizeof(char));
        if (str->string == NULL){
            fprintf(stderr, "Internal Error: Failed to reallocate memory");
            error(INTERNAL_ERROR);
        }
        str->allocated = reallocSize;
    }
    str->curr_len += strlen(append);
    strcat(str->string, append);
    str->string[str->curr_len] = '\0';
}

void dynamic_string_append_char(dstring_t *str, char character){
    if (str->curr_len + 1 >= str->allocated){
        str->string = realloc(str->string, str->allocated * 2 * sizeof(char));
        if (str->string == NULL){
            fprintf(stderr, "Internal Error> Failed to reallocate memory");
            error(INTERNAL_ERROR);
        }
        str->allocated *= 2;
    }
    str->string[str->curr_len] = character;
    str->curr_len++;
    str->string[str->curr_len] = '\0';
}