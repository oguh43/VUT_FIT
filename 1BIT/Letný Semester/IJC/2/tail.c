// tail.c
// Řešení IJC-DU2, příklad 1, 18.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	size_t capacity;
	size_t write_index;
	size_t read_index;
	char **data;
} CircularBuffer;

CircularBuffer cbuf_create(size_t capacity);
char *cbuf_put(CircularBuffer *cb, char *line);
char *cbuf_get(CircularBuffer *cb);
void cbuf_free(CircularBuffer cb);

bool is_numeric(char* str);

#define MAX_LINE_LENGTH ((size_t)2047+2)

bool tail(FILE *input, FILE *output, size_t line_count);
char *read_line(FILE *input, char *buffer, size_t len);

int main(int argc, char **argv) {
	(void)argc;
	
	char *filename = NULL;
	size_t line_count = 10;
	
	while (*++argv) {
		if (strcmp(*argv, "-n") == 0) {
			if (!is_numeric(*++argv)) {
				fprintf(stderr, "Error: Expected unsigned integer!\n");
				return 1;
			}
			char *endptr;
			line_count = strtoul(*argv, &endptr, 10);
			
			if (!is_numeric(*argv)) {
				fprintf(stderr, "Error: Expected unsigned integer!\n");
				return 1;
			}
		} else if (filename) {
			fprintf(stderr, "Error: multiple files specified!\n");
			return 1;
		} else {
			filename = *argv;
		}
	}
	
	FILE *input_file = filename ? fopen(filename, "r") : stdin;
	if (input_file == NULL){
		fprintf(stderr, "Error: Couldn't open file for reading!\n");
		return 1;
	}
	int exit_status = 0;
	
	if (!tail(input_file, stdout, line_count)) {
		fprintf(stderr,"Error: Unknown error occurred while executing!\n");
		exit_status = 1;
	}
	
	if (filename)
		fclose(input_file);
	
	return exit_status;
}

bool tail(FILE *input, FILE *output, size_t line_count) {
	
	CircularBuffer lines = cbuf_create(line_count);
	if (lines.capacity == 0)
		return true;
	char *buffer = NULL;
	
	bool warning = false;
	size_t warning_distance = 0;
	
	for (;; ++warning_distance) {
		if (!buffer) {
			buffer = malloc(MAX_LINE_LENGTH);
			if (!buffer)
				goto bail_out;
		}
		
		char *end = read_line(input, buffer, MAX_LINE_LENGTH);
		if (end == buffer)
			break;
		
		if (end[-1] != '\n' && end - buffer + 1 == MAX_LINE_LENGTH) {
			fscanf(input, "%*[^\n]");
			fscanf(input, "%*c");
			end[-1] = '\n';
			warning = true;
			warning_distance = 0;
		}
		
		buffer = cbuf_put(&lines, buffer);
	}
	
	if (buffer)
		free(buffer);
	while ((buffer = cbuf_get(&lines))) {
		fputs(buffer, output);
		free(buffer);
	}
	cbuf_free(lines);
	
	if (warning && warning_distance <= line_count) {
		fprintf(stderr,"tail: warning: some long lines were trimmed to %zu characters\n",MAX_LINE_LENGTH - 2);
	}
	
	return true;
	
	bail_out:
	if (buffer)
		free(buffer);
	while ((buffer = cbuf_get(&lines)))
		free(buffer);
	
	cbuf_free(lines);
	
	return false;
}

char *read_line(FILE *input, char *buffer, size_t len) {
	
	int c = 0;
	while (--len && c != '\n' && (c = fgetc(input)) != EOF)
		*buffer++ = c;
	*buffer = '\0';
	
	return buffer;
}

CircularBuffer cbuf_create(size_t n) {
	CircularBuffer cb = {
			.capacity = n + 1,
			.write_index = 0,
			.read_index = 0,
			.data = NULL,
	};
	
	if (n < 1) {
		cb.capacity = 0;
		return cb;
	}
	
	cb.data = calloc(n + 1, sizeof(char *));
	if (!cb.data)
		cb.capacity = 0;
	
	return cb;
}

char *cbuf_put(CircularBuffer *cb, char *line) {
	char *ret = (cb->write_index + 1) % cb->capacity == cb->read_index ? cbuf_get(cb) : NULL;
	
	cb->data[cb->write_index] = line;
	cb->write_index = (cb->write_index + 1) % cb->capacity;
	return ret;
}

char *cbuf_get(CircularBuffer *cb) {
	if (cb->read_index == cb->write_index)
		return NULL;
	
	char *ret = cb->data[cb->read_index];
	cb->read_index = (cb->read_index + 1) % cb->capacity;
	
	return ret;
}

void cbuf_free(CircularBuffer cb) {
	if (cb.data)
		free(cb.data);
}

bool is_numeric(char* str) {
	if (str == NULL){
		return false;
	}
	int len = strlen(str);
	for (int i = 0; i < len; i++){
		if (!isdigit(str[i])){
			return false;
		}
	}
	return true;
}
