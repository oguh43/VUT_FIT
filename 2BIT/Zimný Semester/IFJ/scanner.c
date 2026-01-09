/**
 * FIT VUT - IFJ project 2024
 *
 * @file scanner.c
 *
 * @brief Scanner implementation IFJ24
 *
 * @author Hugo Bohácsek (xbohach00)
 * @author Josef Ambruz (xambruj00)
 */

#include "scanner.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "error.h"

#include "ptr_registry.h"

const char *sc_keywords[] = {"const", "else", "fn", "if", "i32", 
							"f64", "null", "pub", "return", "u8", 
							"var", "void", "while", "for", "import"};

const char *sc_operands = "+-*/(){}[];@:.,?|";

const char *sc_builtin[] = {"readstr", "readi32", "readf64", "write", "i2f", "f2i", "string", "length", "concat", "substring", "strcmp", "ord", "chr"};

// Strdup sa mi hodilo pre kopírovanie stringov ale nie je v ISO c :pepega:
char *string_duplicate(const char *src) {
	char *dest = malloc(strlen(src) + 1);
	if (dest == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		error(INTERNAL_ERROR);
	}
	ptr_registry_add(dest);
	strcpy(dest, src);
	return dest;
}

// Funkcia pre resizovanie bufferu. Podľa prednášky vždy 2 * pôvodná veľkosť.
char* resize_buffer(char *buffer, int *current_size) {
	*current_size *= 2;
	buffer = (char*) ptr_registry_realloc((void*) buffer, *current_size);
	return buffer;
}

// Dostali sme keyword? Pripravené na niektoré extendy.
int is_keyword(const char *str) {
	
	for (int i = 0; i < (int) (sizeof(sc_keywords) / sizeof(sc_keywords[0])); ++i) {
		if (strcmp(str, sc_keywords[i]) == 0) {
			return i;
		}
	}
	return -1;
}

bool is_builtin(const char *str) {
	
	for (int i = 0; i < (int) (sizeof(sc_builtin) / sizeof(sc_builtin[0])); ++i) {
		if (strcmp(str, sc_builtin[i]) == 0) {
			return true;
		}
	}
	return false;
}

// Funkcia na ľahšie vytváranie tokenov. Mohlo by byť fajn ako makro?
Token create_token(TokenType type, const char *value) {
	Token token;
	token.type = type;
	token.value = string_duplicate(value);  // Use custom string duplication
	return token;
}

// Spracovanie stringov
Token handle_string(FILE *source) {
	int buffer_size = INITIAL_BUFFER_SIZE;
	char *buffer = malloc(buffer_size);
	int idx = 0;
	int c;

	if (!buffer) {
		// DEBUG - odstráň fprintf
		fprintf(stderr, "Error: Memory allocation failed.\n");
		error(INTERNAL_ERROR);
	}
	ptr_registry_add(buffer);

	while ((c = fgetc(source)) != '"' && c != EOF) {
		if (c == '\\') {
			// Escape sekvencie ako napríklad \" alebo \n // TODO - v c sa veľmi ťažko spracovávajú sekvencie s \ pretože sú to 2 znakové sekvencie
			c = fgetc(source);
			if (c == 'n') {
				buffer[idx++] = '\n';
			} else if (c == 't') {
				buffer[idx++] = '\t';
			} else if (c == 't') {
				buffer[idx++] = '\r';
			} else if (c == '"') {
				buffer[idx++] = '"';
			} else if (c == '\\') {
				buffer[idx++] = '\\';
			} else if (c == 'x') {
				// Objevili jsme hexadecimální escape sekvenci
				char digit1 = fgetc(source);
				char digit2 = fgetc(source);

				if ((digit1 < '0' || digit1 > '9') && 
            		(digit1 < 'A' || digit1 > 'F') &&
					(digit1 < 'a' || digit1 > 'f')) 
        		{  
					fprintf(stderr, "Error: Invalid Hex value"); 
					error(LEXICAL_ERROR); 
        		}
				if ((digit2 < '0' || digit2 > '9') && 
            		(digit2 < 'A' || digit2 > 'F') &&
					(digit2 < 'a' || digit2 > 'f')) 
        		{  
					fprintf(stderr, "Error: Invalid Hex value"); 
					error(LEXICAL_ERROR);
        		}

				char hexString[3] = {digit1, digit2, '\0'};
				int hexValue = (int)strtol(hexString, NULL, 16);

				if (hexValue > 127) { fprintf(stderr, "Error: Invalid ASCII value"); error(LEXICAL_ERROR); }

				buffer[idx++] = (char)hexValue;
			}else{
				// Not a valid escape sequence
				exit(LEXICAL_ERROR);
			}

		} else {
			buffer[idx++] = c;
		}

		if (idx >= buffer_size - 1) {
			buffer = resize_buffer(buffer, &buffer_size);
		}
	}

	if (c == EOF) {
		ptr_registry_remove(buffer);
		return create_token(TOKEN_ERROR, "Unterminated string"); // Chyba tokenu - TODO - error error
	}

	buffer[idx] = '\0';
	Token token = create_token(TOKEN_TP_STRING, buffer);  // String token
	ptr_registry_remove(buffer);
	return token;
}

// Čísla a floaty 🤮
Token handle_number(FILE *source, int c) {
	int buffer_size = INITIAL_BUFFER_SIZE;
	char *buffer = malloc(buffer_size);
	int idx = 0;
	_Bool is_float = false;

	if (!buffer) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		error(INTERNAL_ERROR);
	}
	ptr_registry_add(buffer);

	buffer[idx++] = c;  // prvé číslo sme prečítali už v hlavnej funkcii

	// Čítame celočíselnú časť
	while (isdigit(c = fgetc(source))) {
		buffer[idx++] = c;
		if (idx >= buffer_size - 1) {
			buffer = resize_buffer(buffer, &buffer_size);
		}
	}

	// TODO - asi akceptuje rozbité čísla? VERIFY

	// Overíme, či je float
	if (c == '.') {
		is_float = true;
		buffer[idx++] = c;
		if (idx >= buffer_size - 1) {
			buffer = resize_buffer(buffer, &buffer_size);
		}
		while (isdigit(c = fgetc(source))) {
			buffer[idx++] = c;
			if (idx >= buffer_size - 1) {
				buffer = resize_buffer(buffer, &buffer_size);
			}
		}
	}

	// Čas na exponenty
	if (c == 'e' || c == 'E') {
		is_float = true;
		buffer[idx++] = c;
		if (idx >= buffer_size - 1) {
			buffer = resize_buffer(buffer, &buffer_size);
		}
		c = fgetc(source);
		if (c == '+' || c == '-') {
			buffer[idx++] = c;  // Kladný/ záporný exponent?
			if (idx >= buffer_size - 1) {
				buffer = resize_buffer(buffer, &buffer_size);
			}
			c = fgetc(source);
		}

			// Dočítame zvyšok exponentu
		while (isdigit(c)) {
			buffer[idx++] = c;
			if (idx >= buffer_size - 1) {
				buffer = resize_buffer(buffer, &buffer_size);
			}
			c = fgetc(source);
		}
	}

	ungetc(c, source);  // Charakter, ktorý nás donútil prestať čítať vrátime naspäť na budúce spracovanie.
	buffer[idx] = '\0';

	Token token;
	if (is_float){
		token = create_token(TOKEN_TP_FLOAT, buffer);
		token.typed_value.float_val = atof(buffer);
	} else {
		token = create_token(TOKEN_TP_INT, buffer);
		token.typed_value.int_val = atoi(buffer);
	}
	ptr_registry_remove(buffer);
	return token;
}

// Hlavná funkcia na čítanie zo zdroja
Token get_next_token(FILE *source) {
	int c;
	int buffer_size = INITIAL_BUFFER_SIZE;
	char *buffer = malloc(buffer_size);
	int idx = 0;

	if (!buffer) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		error(INTERNAL_ERROR);
	}
	ptr_registry_add(buffer);

	// Preskakujeme "biele" (kekw) znaky
	while (isspace(c = fgetc(source)));

	// Koniec zdroja // move to the top????
	if (c == EOF) {
		ptr_registry_remove(buffer);
		return create_token(TOKEN_EOF, "EOF"); // Špeciálny token informujúci volajúceho o konci zdroja
	}

	

	// Komentáre preskakujeme
	if (c == '/') {
		int next_char = fgetc(source);
		if (next_char == '/') {
			// Konzumujeme až do konca riadku/ zdroja
			while ((c = fgetc(source)) != '\n' && c != EOF);

			ptr_registry_remove(buffer);
			return get_next_token(source);  // Rovno zavoláme samých seba a snažíme sa prečítať následujúci token
		} else {
			// Ak bolo len jedno lomítko, našli sme operátor delenia
			ungetc(next_char, source);  // Vrátime znak, ktorý sme prečítali po prvom /
			buffer[0] = c;
			buffer[1] = '\0';

			Token token = create_token(TOKEN_OPE_DIV, buffer); // Vraciame "/" ako operátor
			ptr_registry_remove(buffer);
			return token;
		}
	}

	// Operátory porovnávania
	if (c == '<' || c == '>' || c == '=' || c == '!') {
		int next_char = fgetc(source);
		if (c == '='){ // = == 
			if (next_char == '=') {
				buffer[0] = c;
				buffer[1] = next_char;
				buffer[2] = '\0';
				Token token = create_token(TOKEN_OPE_EQ, buffer);
				ptr_registry_remove(buffer);
				return token;
			} else {
				ungetc(next_char, source);
				buffer[0] = c;
				buffer[1] = '\0';
				Token token = create_token(TOKEN_OPE_ASSIGN, buffer);
				ptr_registry_remove(buffer);
				return token;
			}
		} else { // > < ! >= <= !=
			if (next_char != '=') {
				ungetc(next_char, source);
				Token token;
				buffer[0] = c;
				buffer[1] = '\0';
				switch (c) {
					case '<':
						token = create_token(TOKEN_OPE_LT, buffer);
						break;
					case '>':
						token = create_token(TOKEN_OPE_GT, buffer);
						break;
					case '!':
						token = create_token(TOKEN_OPE_NEQ, buffer);
						break;
				}
				ptr_registry_remove(buffer);
				return token;
			}
			buffer[0] = c;
			buffer[1] = next_char;
			buffer[2] = '\0';
			Token token;
			switch (c) {
				case '<':
					token = create_token(TOKEN_OPE_LTE, buffer);
					break;
				case '>':
					token = create_token(TOKEN_OPE_GTE, buffer);
					break;
				case '!':
					token = create_token(TOKEN_OPE_NEQ, buffer);
					break;
			}
			ptr_registry_remove(buffer);
			return token;
		}
	}

	// YAY, string (:
	if (c == '"') {
		ptr_registry_remove(buffer);
		return handle_string(source);
	}

	// Snažíme sa získať identifikátor / kľúčové slovo
	if (isalpha(c) || c == '_') {
		buffer[idx++] = c;
		while (isalnum(c = fgetc(source)) || c == '_') {
			buffer[idx++] = c;
			if (idx >= buffer_size - 1) {
				buffer = resize_buffer(buffer, &buffer_size);
			}
		}
		ungetc(c, source);  // Znak, ktorý nás donútil prestať čítať musíme vrátiť naspäť pre budúce spracovanie
		buffer[idx] = '\0';

		// Podmienka pre zistenie, či vraciame prečítaný buffer ako kľúćové slovo alebo identifikátor
		int n;
		if ((n = is_keyword(buffer)) >= 0) {
			Token token = create_token(n + KW_OFFSET, buffer);
			ptr_registry_remove(buffer);
			return token;
		} else if(is_builtin(buffer)){
			Token token = create_token(TOKEN_BUILTIN, buffer);
			ptr_registry_remove(buffer);
			return token;
		}else {
			Token token = create_token(TOKEN_ID, buffer);
			ptr_registry_remove(buffer);
			return token;
		}
		
	}

	// Možno číslo / float?
	if (isdigit(c)) {
		ptr_registry_remove(buffer);
		return handle_number(source, c);
	}

	// Jednoznakové operátory // VERIFY - kuknúť čo všetko tu smie byť / či mám rozdeliť na rôzne druhy tokenov
	char *ret;
	
	if ((ret = strchr(sc_operands, c)) != NULL) { // "+-*/(){}[];@:.?"
		buffer[0] = c;
		buffer[1] = '\0';
		Token token = create_token(OPE_OFFSET+(ret - sc_operands), buffer);
		ptr_registry_remove(buffer);
		return token;
	}

	// Nevalídny vstup :(
	ptr_registry_remove(buffer);
	exit(LEXICAL_ERROR);
	//return create_token(TOKEN_ERROR, "Invalid token");
}
