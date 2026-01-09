/**
 * FIT VUT - IFJ project 2024
 *
 * @file dynamic_string.h
 *
 * @brief Dynamic string implementation for code generator
 *
 * @author Filip Jenis (xjenisf00)
 */

#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <string.h>

#define DYNAMIC_STRING_INIT_SIZE 10

typedef struct {
    char *string;
    unsigned curr_len;
    unsigned allocated;
} dstring_t;


/**
 * Initialization of dynamic string struct
 *
 * @param str Pointer to dynamic string struct
 */
void dynamic_string_init(dstring_t *str);

/**
 * Free allocated memory for dynamic string struct
 *
 * @param str Pointer to dynamic string struct
 */
void dynamic_string_dispose(dstring_t *str);

/**
 * Append string to the end of the dynamic string
 *
 * @param str Pointer to dynamic string struct
 * @param append Pointer to const string to be appended
 */
void dynamic_string_append(dstring_t *str, const char *append);

/**
 * Append character to the end of the dynamic string
 * @param str Pointer to dynamic string struct
 * @param character Character to be appended
 */
void dynamic_string_append_char(dstring_t *str, char character);

#endif //DYNAMIC_STRING_H
