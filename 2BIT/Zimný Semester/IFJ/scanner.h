/**
 * FIT VUT - IFJ project 2024
 *
 * @file scanner.h
 *
 * @brief Scanner implementation IFJ24
 *
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdbool.h>

#define INITIAL_BUFFER_SIZE 128
// Enum to define different token types
typedef enum {

	TOKEN_KW_CONST,
	TOKEN_KW_ELSE,
	TOKEN_KW_FN,
	TOKEN_KW_IF,
	TOKEN_KW_I32,
	TOKEN_KW_F64,
	TOKEN_KW_NULL,
	TOKEN_KW_PUB,
	TOKEN_KW_RETURN,
	TOKEN_KW_U8,
	TOKEN_KW_VAR,
	TOKEN_KW_VOID,
	TOKEN_KW_WHILE,
	TOKEN_KW_FOR,
	TOKEN_KW_IMPORT,

	TOKEN_ID,

	TOKEN_TP_INT,
	TOKEN_TP_FLOAT,
	TOKEN_TP_STRING,

	TOKEN_OPE_ADD,//20
	TOKEN_OPE_SUB,
	TOKEN_OPE_MUL,
	TOKEN_OPE_DIV,
	TOKEN_OPE_LPAREN,
	TOKEN_OPE_RPAREN,
	TOKEN_OPE_LBRACE, // treba?
	TOKEN_OPE_RBRACE,	// treba?
	TOKEN_OPE_LBRACKET,
	TOKEN_OPE_RBRACKET,
	TOKEN_OPE_SEMICOLON,
	TOKEN_OPE_AT,
	TOKEN_OPE_COLON,
	TOKEN_OPE_DOT,
	TOKEN_OPE_COMMA,
	TOKEN_NULLABLE,
	TOKEN_PIPE,

	TOKEN_OPE_ASSIGN,
	TOKEN_OPE_EQ,
	TOKEN_OPE_NEQ,
	TOKEN_OPE_LT,
	TOKEN_OPE_GT,
	TOKEN_OPE_LTE,
	TOKEN_OPE_GTE,

	TOKEN_EOF,
	TOKEN_ERROR,

	TOKEN_BUILTIN,
} TokenType;

#define KW_OFFSET 0
// 20?????
#define OPE_OFFSET 19

typedef enum {
    TYPE_UNDEFINED,
    TYPE_I32,
    TYPE_F64,
    TYPE_U8,
    TYPE_SLITERAL,
    TYPE_FVOID
} DataType;

typedef struct STData {
    DataType DataType;
    bool nullable;
    bool slice;
} STData;

// Structure to represent a token
typedef struct {
	TokenType type;
	STData data;
	bool is_e;
	char *value;
	union {
		int int_val;
		float float_val;
	} typed_value;
} Token;

#define GET_TYPED_VALUE(token) (\
	((token).type == TOKEN_TP_INT)?(token).typed_value.int_val:\
	((token).type == TOKEN_TP_FLOAT)?(token).typed_value.float_val:(token).value\
)

extern const char *sc_keywords[];
extern const char *sc_operands;
extern const char *sc_builtin[];

// Function declarations
/**
 * @brief Duplicates a string
 * 
 * @param src Source string
 * @return char* Duplicated string
 */
char *string_duplicate(const char *src);

/**
 * @brief Resizes a buffer
 * 
 * @param buffer Buffer to resize
 * @param current_size Current size of the buffer
 * @return char* Resized buffer
 */
char* resize_buffer(char *buffer, int *current_size);

/**
 * @brief Checks if a string is a keyword
 * 
 * @param str String to check
 * @return index of string in keyword list if the string is a keyword
 * @return -1 If the string is not a keyword
 */
int is_keyword(const char *str);

/**
 * @brief Checks if a string is an operand
 * 
 * @param str String to check
 * @return true If the string is an operand
 * @return false If the string is not an operand
 */
bool is_builtin(const char *str);

/**
 * @brief Checks if a string is an operand
 * 
 * @param str String to check
 * @return true If the string is an operand
 * @return false If the string is not an operand
 */
Token create_token(TokenType type, const char *value);

/**
 * @brief Checks if a string is an operand
 * 
 * @param str String to check
 * @return true If the string is an operand
 * @return false If the string is not an operand
 */
Token get_next_token(FILE *source);

#endif // SCANNER_H
