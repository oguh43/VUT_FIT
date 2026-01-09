/**
 * FIT VUT - IFJ project 2024
 *
 * @file main.h
 *
 * @brief Main function
 *
 * @author Hugo Bohácsek (xbohach00)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ptr_registry.h"
#include "scanner.h"
#include "parser.h"

void debug_token_print(Token token, FILE *stream, bool NL);

int main(int argc, char *argv[]){


	FILE *input = stdin;
	Token token;

	if (atexit(ptr_registry_cleanup) != 0){
		fprintf(stderr, "Failed to set up cleaning function"); // DEBUG // QUESTION - interný error prekladača???
	}

	ptr_registry_init();
	
	
	parser_parse(input);

	ptr_registry_cleanup();
	fclose(input);

	return 0;
}

void debug_token_print(Token token, FILE *stream, bool NL) {
	return;
	const char *types[] = {"TOKEN_KW_CONST", "TOKEN_KW_ELSE", "TOKEN_KW_FN", "TOKEN_KW_IF", "TOKEN_KW_I32", "TOKEN_KW_F64", "TOKEN_KW_NULL", "TOKEN_KW_PUB", "TOKEN_KW_RETURN", "TOKEN_KW_U8", "TOKEN_KW_VAR", "TOKEN_KW_VOID", "TOKEN_KW_WHILE", "TOKEN_KW_FOR", "TOKEN_KW_IMPORT", "TOKEN_ID", "TOKEN_TP_INT", "TOKEN_TP_FLOAT", "TOKEN_TP_STRING", "TOKEN_OPE_ADD", "TOKEN_OPE_SUB", "TOKEN_OPE_MUL", "TOKEN_OPE_DIV", "TOKEN_OPE_LPAREN", "TOKEN_OPE_RPAREN", "TOKEN_OPE_LBRACE", "TOKEN_OPE_RBRACE", "TOKEN_OPE_LBRACKET", "TOKEN_OPE_RBRACKET", "TOKEN_OPE_SEMICOLON", "TOKEN_OPE_AT", "TOKEN_OPE_COLON", "TOKEN_OPE_DOT", "TOKEN_OPE_COMMA", "TOKEN_NULLABLE", "TOKEN_PIPE", "TOKEN_OPE_ASSIGN", "TOKEN_OPE_EQ", "TOKEN_OPE_NEQ", "TOKEN_OPE_LT", "TOKEN_OPE_GT", "TOKEN_OPE_LTE", "TOKEN_OPE_GTE", "TOKEN_EOF", "TOKEN_ERROR", "TOKEN_BUILTIN"};
	char modified_value[200];
	int j = 0;
	for (int i = 0; token.value[i] != '\0'; i++) {
		if (token.value[i] == '\n') {
			modified_value[j++] = '\\';
			modified_value[j++] = 'n';
		} else {
			modified_value[j++] = token.value[i];
		}
	}
	modified_value[j] = '\0';
	fprintf(stream, "[\"TokenType\": \"%s\", \"char\": \"%s\"]", types[token.type], modified_value);
	if (NL) {
		fprintf(stream, "\n");
	}
}


