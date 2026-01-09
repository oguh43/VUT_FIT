/**
 * FIT VUT - IFJ project 2024
 *
 * @file error.h
 *
 * @brief Error exit handling
 *
 * @author Hugo Bohácsek (xbohach00)
 */

#include <err.h>

enum error_type {
    LEXICAL_ERROR = 1,
    SYNTAX_ERROR,
    SEMANTIC_ERROR_UNDEFINED,
    SEMANTIC_ERROR_FUNC,
    SEMANTIC_ERROR_REDEFINE,
    SEMANTIC_ERROR_MISSING,
    SEMANTIC_ERROR_INCOMPAT,
    SEMANTIC_ERROR_TYPE,
    SEMANTIC_ERROR_UNUSED,
    SEMANTIC_ERROR_OTHER,
    INTERNAL_ERROR = 99
};

/**
 * @brief Print error message and exit with error code
 * 
 * @param num Error code
 */

void error(int num);
