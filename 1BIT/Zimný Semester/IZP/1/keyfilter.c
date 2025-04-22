//
// Created by xbohach00 on 4. 10. 2023.
//
// Stream implementation
#define N_CHARS 120
#define ASCII_SIZE 128

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

char* capitalize(char* string);

int main(int argc, char** argv){
	
	char find[N_CHARS] = {'\0'};
	if (argc > 1){
		if (strlen(argv[1]) > N_CHARS){
			fprintf(stderr, "Maximum search lenght exceeded! Maximum allowed is 100!");
			return 1;
		}
		strcpy(find, capitalize(argv[1]));
	}
	
	int ASCII_map[ASCII_SIZE] = {0};
	
	char line[N_CHARS]; // A line should have max 100 lines, lets read more just in case
	char lastMatch[N_CHARS] = {'\0'};
	bool notEmpty = false;
	bool missedNL = false;
	
	while (fgets(line, N_CHARS, stdin)){ //Reading from stream redirect
		notEmpty = true;
		if (missedNL == true){
			fprintf(stderr, "Line too long! Maximum line length is 100!");
			return 1;
		}
		if (line[strlen(line) - 1] == 10){ //Remove newline character from a line
			line[strlen(line)-1] = 0;
		}else{
			missedNL = true;					//detect if a line is longer than 100 characters
		}
		strcpy(line, capitalize(line));

		if (strncmp(find, line, strlen(find)) != 0){
			continue;
		}
		
		ASCII_map[(int) line[strlen(find) == strlen(line)? strlen(find)-1: strlen(find)]] += 1;
		strcpy(lastMatch, line);
	}
	
	if (notEmpty == false){
		fprintf(stderr, "No input adresses were provided!");
		return 1;
	}
	
	char result[ASCII_SIZE] = {'\0'}; //Reorder result to be in alphabetical order
	int mx = 0;
	for (int index = 0; index < ASCII_SIZE; index++){
		if (ASCII_map[index] > 0){
			result[strlen(result)] = (char) index;
			if (ASCII_map[index] > mx){
				mx = ASCII_map[index];
			}
		}
	}
	
	
	
	if (strlen(result) == 0){ //Print our result
		printf("Not found");
	}else if (strlen(result) == 1 && mx == 1){
		printf("Found: %s", lastMatch);
	} else{
		printf("Enable: %s", result);
	}
	
	return 0;
}

char* capitalize(char* string){ // Change string to uppercase
	for (int i=0; (unsigned) i < strlen(string); i++){
		if (string[i] >= 97 && string[i] <= 122){
			string[i] -= 32;
		}
	}
	return string;
}
