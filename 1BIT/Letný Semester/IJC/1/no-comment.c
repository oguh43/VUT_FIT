// no-comment.c
// Řešení IJC-DU1, příklad b), 20.3.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20230801

#include <stdio.h> // removed string.h
#include <stdbool.h>
#include <ctype.h>
#include "error.h"

enum eState{
	NONE,
	LINE,
	MULTILINE
};


int main(int argc, char** argv){
	FILE* fp = NULL;


	if (argc == 2){
		fp = fopen(argv[1], "r");
        if (fp == NULL){
            error_exit("Error opening file %s", argv[1]);
        }
	}
	if (fp == NULL){
		fp = stdin;
	}

	enum eState state = NONE;
	int cur_char, prev_char;
    prev_char = -1;
	int str_char = -1;
	bool in_string = false;
	bool prev_char_escaped = false;
	bool wrap_line = false;
	int store = -1;
	while((cur_char = fgetc(fp)) != EOF){
        if (wrap_line == false){store=-1;}
		if (wrap_line && isspace(cur_char) != 0){
			continue;
		}else if (wrap_line && cur_char == '\\'){		// a very **very** bad hotfix for not printing \\ :D
			wrap_line = false;
			prev_char = store;
			printf("%c", prev_char);
			prev_char = '\\';
		}else if (wrap_line){
			prev_char = store;
			wrap_line = false;
			store = '\\';
		}
		if (prev_char == '\\' && cur_char == '\\' && !prev_char_escaped){
			prev_char_escaped = true;
		}else if(cur_char == '\\' && !in_string && state != LINE){
			wrap_line = true;
			store = prev_char;
			prev_char = -1;
			continue;
		}
        if (state == NONE && (cur_char == '\"' || cur_char == '\'')){
            if (prev_char != '\\' || prev_char_escaped) {
				prev_char_escaped = false; 								// ????? PROBLEMS????
				if (in_string){
					if (cur_char == str_char) {
						in_string = false;
					}
				}else{
					in_string = true;
					str_char = cur_char;
				}
            }
        }
		//printf("%d \\%c\\", in_string, str_char);
        if (prev_char == '/' && cur_char == '/'){
            if (!in_string) {
                state = LINE;
            }
        }
        if (prev_char == '/' && cur_char == '*' && state == NONE){
            if (!in_string){
                state = MULTILINE;
            }
        }
        if (state == MULTILINE && prev_char == '*' && cur_char == '/'){
            state = NONE;
            prev_char = -1;
            cur_char = -1;
        }
        if (state == LINE && cur_char == '\n') {
            if (prev_char == '\\') { continue; }
            state = NONE;
            printf("\n");
            prev_char = -1;
            continue;
        }
        if (state == NONE && prev_char >= 0){
            if (store != -1){
                printf("%c%c\n", prev_char, store);
            }else {
                printf("%c", prev_char);
            }
        }
        prev_char = cur_char;
	}
    if (prev_char > 0 && state != LINE){
        printf("%c", prev_char);
    }

    if (state == MULTILINE){
        error_exit("Multiline comment not closed!");
        fprintf(stderr, "ERROR, MULTILINE COMMENT NOT CLOSED!!!");
    }
    return 0;
}
