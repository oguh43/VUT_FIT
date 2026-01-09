/**
 * @file inp_gen.c
 * @brief Generates input for standard deviation.
 * @author Hugo Bohácsek (xbohach00)
 */
//gcc -o inp_gen -O3 inp_gen.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Usage: %s <min> <max> <ammount>\n", argv[0]);
		return 1;
	}

	float min = atof(argv[1]);
	float max = atof(argv[2]);
	if (min >= max) {
		printf("Error: min must be less than max\n");
		return 1;
	}

	int ammount = atoi(argv[3]);
	if (ammount <= 0){
		printf("Error: ammount must be greater than 0\n");
		return 1;
	}
	srand(time(NULL));

	// Generate and print random floats within the specified range

	for (int i = 0; i < ammount; i++) {
		float randomFloat = min + ((float) rand() / RAND_MAX) * (max - min);
		printf("%f\n", randomFloat);
	}

	return 0;
}