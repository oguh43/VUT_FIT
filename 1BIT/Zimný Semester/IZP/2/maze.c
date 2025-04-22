// 12.1.2023 - xbohach00
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
enum mode{
	INVALID,
	HELP,
	TEST,
	LPATH,
	RPATH,
	SHORTEST
};
enum test_e{
	test_VALID,
	test_INVALID,
	test_FILE,
	test_MALLOC
};
enum wall_e{
	wall_LEFT,
	wall_RIGHT,
	wall_UP_DOWN
};
enum wall_rules_rpath{
	RIGHT_1 = 6,
	DOWN_2 = 5,
	LEFT_3 = 2,
	RIGHT_4 = 1,
	UP_5 = 4,
	LEFT_6 = 3
};
enum wall_rules_lpath{
	l_UP_1 = 6,
	l_RIGHT_2 = 5,
	l_RIGHT_3 = 2,
	l_LEFT_4 = 1,
	l_LEFT_5 = 4,
	l_UP_6 = 3
};
typedef struct {
	int rows;
	int cols;
	unsigned char *cells;
} Map;
typedef struct {
	char *name;
	int len;
	int r;
	int c;
} file_t;
typedef struct{
	int r;
	int c;
} exitDetails;
/**
 * @brief Handles command line arguments and sets the program mode.
 * @param argc Number of command line arguments.
 * @param argv Array of command line argument strings.
 * @param file File structure to store file-related information.
 * @return Program mode (enum mode).
 */
enum mode handleArg(int argc, char**argv, file_t *file);
/**
 * @brief Initializes the maze from a file.
 * @param maze Pointer to the maze structure.
 * @param fileArg Pointer to the file structure.
 * @return Loading result (enum test_e).
 */
enum test_e initMaze(Map *maze, file_t *fileArg);
/**
 * @brief Consumes characters from a file stream and extracts a number.
 * @param fptr File pointer.
 * @param num Pointer to store the extracted number.
 * @param multiple Flag indicating if multiple digits are expected.
 * @return True if successful, false otherwise.
 */
bool consumeStream(FILE *fptr, int* num, bool multiple);
/**
 * @brief Deallocates memory pointed by the given pointer.
 * @param ptr Pointer to the memory to be deallocated.
 */
void destroy(void* ptr);
/**
 * @brief Checks a specific bit in a number. Used in isborder.
 * @param num The number to check.
 * @param pos The position of the bit to check.
 * @return True if the bit is set, false otherwise.
 */
bool checkBit( int num, int pos);
/**
 * @brief Calculates the power of a number.
 * @param num Base number.
 * @param exp Exponent.
 * @return The result of num raised to the power of exp.
 */
int poww(int num, int exp);
/**
 * @brief Validates the maze structure.
 * @param maze Pointer to the maze structure.
 * @return Test result (enum test_e).
 */
enum test_e validateMaze(Map *maze);
/**
 * @brief Checks if a specific border of a cell in the maze is solid.
 * @param map Pointer to the maze structure.
 * @param r Row index.
 * @param c Column index.
 * @param border Wall (enum wall_e).
 * @return True if the border is set, false otherwise.
 */
bool isborder(Map *map, int r, int c, int border);
/**
 * @brief Determines the starting border based on entry parameters (r, c) and hand rule (enum action).
 * @param map Pointer to the maze structure.
 * @param r Row index.
 * @param c Column index.
 * @param leftright Path direction (LPATH or RPATH).
 * @return Starting wall direction (enum wall_e).
 */
int start_border(Map *map, int r, int c, int leftright);
/**
 * @brief Prints the given indexes.
 * @param r Row index.
 * @param c Column index.
 */
void print_path(int r, int c);
/**
 * @brief Determines whether the cell is facing upwards.
 * @param r Row index.
 * @param c Column index.
 * @return 0 if the cell is facing upwards, else 1.
 */
int upOrDown(int r, int c);
/**
 * @brief Moves the current position based on the wall direction and updates the ass.
 * @param r Pointer to the current row index.
 * @param c Pointer to the current column index.
 * @param wall Wall direction (enum wall_e).
 * @param ass Pointer to ass location variable.
 */
void move(int* r, int* c, int wall, int* ass);
/**
 * @brief Gets an array of weighted ass positions, based on the current ass, up position, and left/right rule.
 * @param weights Array to store the weighted ass locations.
 * @param ass Current ass location.
 * @param up If cell is facing upwards (0 or 1).
 * @param leftright Left/right pathfinding rule (LPATH or RPATH).
 */
void getWeightAss(int weights[], int ass, int up, int leftright);
/**
 * @brief Gets ass location based on the heading (hand position) and up position.
 * @param ass Pointer to the ass location variable.
 * @param heading Current heading (wall direction).
 * @param up If cell is facing upwards (0 or 1).
 */
void getAss(int* ass, int heading, int up);
/**
 * @brief Pathfind through the maze, using l/r rules.
 * @param maze Pointer to the maze structure.
 * @param r Starting row.
 * @param c Starting column.
 * @param doPrint Whether or not to print path.
 * @param safe Whether to protect against loops.
 * @param exitD logs exit data.
 * @return Ammount of steps taken.
*/
int pathFind(Map *maze, int action, int r, int c, bool doPrint, bool safe, exitDetails *exitD);
/**
 * @brief Finds the shortest path through a maze.
 * @param r Starting row.
 * @param c Starting column
 * @param maze Pointer to the maze structure.
*/
void shortest(int r, int c, Map *maze);
/**
 * @brief Reconstruct the maze to not include any dead ends.
 * @param maze Pointer to the maze structure.
 * @return Whether any modifications were made
*/
bool reMake(Map *maze);
/**
 * @brief Return a new copy of maze.
 * @param origin Map to be copied.
 * @return Copy of the map provided.
*/
Map copyMap(Map origin);

int main(int argc, char** argv) {
	if (argc == 1){
		fprintf(stderr, "Error, no arguments were given!");
		return 1;
	}
	enum mode action = INVALID;
	file_t file;
	action = handleArg(argc, argv, &file);
	if (action == INVALID){
		fprintf(stderr, "Invalid input! Use --help if unsure!");
		destroy(file.name);
		return 1;
	} else if(action == HELP){
		printf("This is a program to find a path through a maze. Usage:\n--help -> Prints this help message.\n--test [file] -> Validates maze stored in [file]. Prints \'Valid\' on success, otherwise \'Invalid\'\n--lpath [r] [c] [file] Finds a path through the maze stored in [file] using the left hand rule. Enters via cell at [r] [c].\n--rpath [r] [c] [file] Finds a path through the maze stored in [file] using the right hand rule. Enters via cell at [r] [c].\n");
		return 0;
	}
	Map maze;
	maze.cells = NULL;
	enum test_e result;
	result = initMaze(&maze, &file);
	if (result == test_FILE){
		fprintf(stderr,"Error reading file!");
		destroy(file.name);
		destroy(maze.cells);
		return 1;
	}else if (result == test_MALLOC){
		fprintf(stderr,"malloc has failed!");
		destroy(file.name);
		destroy(maze.cells);
		return 1;
	}
	if (result == test_INVALID){
		printf("Invalid\n");
		destroy(file.name);
		destroy(maze.cells);
		return 1;
	}
	
	result = validateMaze(&maze);
	if (result != test_VALID){
		destroy(file.name);
		destroy(maze.cells);
		printf("Invalid\n");
		return 1;
	}
	if (action == TEST){// 9
		printf("Valid\n");
		destroy(file.name);
		destroy(maze.cells);
		return 0;
	}
	if (action == SHORTEST){
		shortest(file.r, file.c, &maze);
		destroy(file.name);
		destroy(maze.cells);
		return 0;
	}
	exitDetails e;
	if (pathFind(&maze, action, file.r, file.c, true, false, &e) == -1){
		destroy(file.name);
		destroy(maze.cells);
		return 1;
	}
	if (file.name != NULL){
		destroy(file.name);
	}
	if (maze.cells != NULL){
		destroy(maze.cells);
	}
	return 0;
}

int pathFind(Map *maze, int action, int r, int c, bool doPrint, bool safe, exitDetails *exitD){
	int currentBorder = start_border(maze, r, c, action);
	int maxSteps = maze->rows * maze->cols * 3;
	int dbg = 0;
	int ass;
	if (action == RPATH){
		getAss(&ass, currentBorder, upOrDown(r,c));
	}else{
		ass = currentBorder;
	}
	
	int stepCount = 0;
	int weightAss[3] = {0,0,0};
	while(1){
		if (safe == true){
			maxSteps--;
		}
		if (maxSteps <= 0){
			return -2;
		}
		exitD->r = r;
		exitD->c = c;
		if (doPrint == true){
			print_path(r,c);
		}
		getWeightAss(weightAss, ass, upOrDown(r,c), action);
		dbg = 0;
		for (int i=0; i < 3; i++){
			if (isborder(maze, r, c, weightAss[i]) == false){
				move(&r, &c, weightAss[i], &ass);
				stepCount++;
				dbg--;
				break;
			}
			dbg++;
		}
		if (dbg==3){
			//fprintf(stderr, "Neni cesta");
			return -1;
		}
		
		if (r >= maze->rows || c >= maze->cols || r < 0 || c < 0){
			break;
		}
	}
	return stepCount;
}
Map copyMap(Map origin){
	Map dest;
	dest.cells = malloc(sizeof(int) * origin.rows * origin.cols);
	dest.cols = origin.cols;
	dest.rows = origin.cols;
	int cnt=0;
	
	while (cnt < origin.cols * origin.rows){
		dest.cells[cnt] = origin.cells[cnt];
		cnt++;
	}
	
	return dest;
}
void shortest(int r, int c, Map *maze){
	int minCnt = maze->rows*maze->cols * 3;
	Map minMaze = copyMap(*maze);

	int minMethod = -1;
	int lp;
	int rp;
	int first = 0;
	exitDetails exitDL;
	exitDetails exitDR;
	while(1){
		while (1){
			if (reMake(maze) == false){
				break;
			}
		}
		
		if (first > 0){

			if (upOrDown(r, c)==0 && r == 0){maze -> cells[c + maze->cols * r] |= 1 << wall_UP_DOWN;}
			if (maze -> cols == c+1){maze -> cells[c + maze->cols * r] |= 1 << wall_RIGHT;}
			if (c == 0){maze -> cells[c + maze->cols * r] |= 1 << wall_LEFT;}
			if (upOrDown(r, c) == 1 && r + 1 == maze -> rows){maze -> cells[c + maze->cols * r] |= 1 << wall_UP_DOWN;}
			}
		lp = pathFind(maze, LPATH, r, c, false, true, &exitDL);
		rp = pathFind(maze, LPATH, r, c, false, true, &exitDR);

		if (first > 0){
			
			if (upOrDown(r, c)==0 && r == 0){maze -> cells[c + maze->cols * r] &= ~(1 << wall_UP_DOWN);}
			if (maze -> cols == c+1){maze -> cells[c + maze->cols * r] &= ~(1 << wall_RIGHT);}
			if (c == 0){maze -> cells[c + maze->cols * r] &= ~(1 << wall_LEFT);}
			if (upOrDown(r, c) == 1 && r + 1 == maze -> rows){maze -> cells[c + maze->cols * r] &= ~(1 << wall_UP_DOWN);}

		}

		if (lp == -2 || rp == -2){break;}
		if (lp == -1 || rp == -1){break;}
		if (lp > rp){
			if (rp < minCnt){
				minCnt = rp;
				destroy(minMaze.cells);
				minMaze = copyMap(*maze);
				minMethod = RPATH;
			}
		} else {
			if (lp < minCnt){
				minCnt = lp;
				destroy(minMaze.cells);
				minMaze = copyMap(*maze);
				minMethod = LPATH;
			}
		}
		
		if (upOrDown(exitDL.r, exitDL.c)==0 && exitDL.r == 0){maze -> cells[exitDL.c + maze->cols * exitDL.r] |= 1 << wall_UP_DOWN;}
		if (maze -> cols == exitDL.c+1){maze -> cells[exitDL.c + maze->cols * exitDL.r] |= 1 << wall_RIGHT;}
		if (exitDL.c == 0){maze -> cells[exitDL.c + maze->cols * exitDL.r] |= 1 << wall_LEFT;}
		if (upOrDown(exitDL.r, exitDL.c) == 1 && exitDL.r + 1 == maze -> rows){maze -> cells[exitDL.c + maze->cols * exitDL.r] |= 1 << wall_UP_DOWN;}
	
		if (upOrDown(exitDR.r, exitDR.c)==0 && exitDR.r == 0){maze -> cells[exitDR.c + maze->cols * exitDR.r] |= 1 << wall_UP_DOWN;}
		if (maze -> cols == exitDR.c+1){maze -> cells[exitDR.c + maze->cols * exitDR.r] |= 1 << wall_RIGHT;}
		if (exitDR.c == 0){maze -> cells[exitDR.c + maze->cols * exitDR.r] |= 1 << wall_LEFT;}
		if (upOrDown(exitDR.r, exitDR.c) == 1 && exitDR.r + 1 == maze -> rows){maze -> cells[exitDR.c + maze->cols * exitDR.r] |= 1 << wall_UP_DOWN;}

		first++;
		
	}
	if (lp >0 && rp < 0){
		if (lp < minCnt){
			pathFind(maze, LPATH, r, c, true, false, &exitDL);
			destroy(minMaze.cells);
			return;
		}
	}
	if (rp > 0 && lp < 0){
		if (rp < minCnt){
			pathFind(maze, RPATH, r, c, true, false, &exitDR);
			destroy(minMaze.cells);
			return;
		}
	}

	pathFind(&minMaze, minMethod, r, c, true, false, &exitDL);
	destroy(minMaze.cells);
	return;

}
bool reMake(Map *maze){
	int cnt = 0;
	int borders = 0;
	int missing = -1;
	for (int row = 0; row < maze -> rows; row++){
		for (int col = 0; col < maze -> cols; col++) {
			for (int wall = 0; wall < 3; wall++){
				if (isborder(maze, row, col, wall) == true){
					borders++;
				}else{
					missing = wall;
				}
			}
			if (borders == 2 && missing != -1){
				if (missing == wall_UP_DOWN && upOrDown(row, col)==0 && row == 0){continue;}
				if (missing == wall_RIGHT && maze -> cols == col+1){continue;}
				if (missing == wall_LEFT && col == 0){continue;}
				if (missing == wall_UP_DOWN && upOrDown(row, col) == 1 && row + 1 == maze -> rows){continue;}
				maze -> cells[col + maze->cols * row] |= 1 << missing;
				if (missing == wall_UP_DOWN){
					if (upOrDown(row, col) == 0){
						if (row == 0){continue;}
						maze -> cells[col + maze->cols * (row-1)] |= 1 << wall_UP_DOWN;
					}else{
						if (row + 1 == maze -> rows){continue;}
						maze -> cells[col + maze->cols * (row+1)] |= 1 << wall_UP_DOWN;
					}
				}else if (missing == wall_RIGHT){
					if (maze -> cols == col + 1){continue;}
					maze -> cells[col+1 + maze->cols * row] |= 1 << wall_LEFT;
				}else {
					if (col == 0){continue;}
					maze -> cells[col-1 + maze->cols * row] |= 1 << wall_RIGHT;
				}
				cnt = cnt+1;
			}
			borders = 0;
			missing = -1;
		}
	}
	if (cnt == 0){
		return false;
	}
	return true;
}

void move(int* r, int* c, int wall, int* ass){
	if (wall == wall_RIGHT){
		*c += 1;
		*ass = wall_LEFT;
	}else if (wall == wall_LEFT){
		*c -= 1;
		*ass = wall_RIGHT;
	}else{
		if (upOrDown(*r, *c) == 0){
			*r -= 1;
		}else {
			*r += 1;
		}
		*ass = wall_UP_DOWN;
	}
}
void getAss(int* ass, int heading, int up){
	if (up==0){
		if (heading == wall_RIGHT){
			*ass = wall_LEFT;
		}else if (heading == wall_LEFT){
			*ass = wall_UP_DOWN;
		}else{
			*ass = wall_RIGHT;
		}
	}else{
		if (heading == wall_RIGHT){
			*ass = wall_UP_DOWN;
		}else if (heading == wall_LEFT){
			*ass = wall_RIGHT;
		}else{
			*ass = wall_LEFT;
		}
	}
}
void getWeightAss(int weights[], int ass, int up, int leftright){
	if (ass == wall_RIGHT){
		if (up == 0){
			weights[0] = (leftright == RPATH)?wall_UP_DOWN: wall_LEFT;
			weights[1] = (leftright == RPATH)?wall_LEFT: wall_UP_DOWN;
		}else{
			weights[0] = (leftright == RPATH)?wall_LEFT: wall_UP_DOWN;
			weights[1] = (leftright == RPATH)?wall_UP_DOWN: wall_LEFT;
		}
		weights[2] = wall_RIGHT;
	}else if (ass == wall_LEFT){
		if (up == 0){
			weights[0] = (leftright == RPATH)?wall_RIGHT: wall_UP_DOWN;
			weights[1] = (leftright == RPATH)?wall_UP_DOWN: wall_RIGHT;
		}else{
			weights[0] = (leftright == RPATH)?wall_UP_DOWN: wall_RIGHT;
			weights[1] = (leftright == RPATH)?wall_RIGHT: wall_UP_DOWN;
		}
		weights[2] = wall_LEFT;
	}else{
		if (up == 0){
			weights[0] = (leftright == RPATH)?wall_LEFT: wall_RIGHT;
			weights[1] = (leftright == RPATH)?wall_RIGHT: wall_LEFT;
		}else{
			weights[0] = (leftright == RPATH)?wall_RIGHT: wall_LEFT;
			weights[1] = (leftright == RPATH)?wall_LEFT: wall_RIGHT;
		}
		weights[2] = wall_UP_DOWN;
	}
}

int start_border(Map *map, int r, int c, int leftright /* LPATH or RPATH*/){
	int rules[6] = {0,0,0,0,0,0};
	int index = 0;
	
	if ( (c == 0) && (r % 2 == 0) ){ // 1st rule
		rules[index] = (leftright == RPATH)?RIGHT_1:l_UP_1;
		index++;
	}
	if ( (c == 0) && (r % 2 == 1) ){ // 2nd rule
		rules[index] = (leftright == RPATH)?DOWN_2:l_RIGHT_2;
		index++;
	}
	if (r == 0){ // 3rd rule
		rules[index] = (leftright == RPATH)?LEFT_3:l_RIGHT_3;
		index++;
	}
	if ( r + 1 == map -> rows ){ // 4th rule
		rules[index] = (leftright == RPATH)?RIGHT_4:l_LEFT_4;
		index++;
	}
	if ( (c + 1 == map -> cols) && (upOrDown(r,c) == 0 ) && (r % 2 == 0)){ // 5th rule
		rules[index] = (leftright == RPATH)?UP_5:l_LEFT_5;
		index++;
	}
	if ( (c == map -> cols) && (upOrDown(r,c) == 1 ) && (r % 2 == 1)){ // 6th rule
		rules[index] = (leftright == RPATH)?LEFT_6:l_UP_6;
		index++;
	}
	
	int res = rules[0];
	if (index > 0){
		for (int i=0; i < index; i++){
			if (rules[i] > res){
				res = rules[i];
			}
		}
	}
	if (leftright == RPATH){
		switch (res) {
			case RIGHT_1:
			case RIGHT_4:
				return wall_RIGHT;
			case LEFT_3:
			case LEFT_6:
				return wall_LEFT;
			case DOWN_2:
			case UP_5:
				return wall_UP_DOWN;
			default:
				return wall_UP_DOWN;
		}
	}else {
		if (index == 2){
			if ((rules[0] == 6 || rules[1] == 6) && (rules[0] == 2|| rules[1] == 2)){
				return wall_UP_DOWN;
			}
		}
		switch (res) {
			case RIGHT_1:
				return wall_LEFT;
			case RIGHT_4:
				return wall_UP_DOWN;
			case LEFT_3:
				return wall_UP_DOWN;
			case LEFT_6:
				return wall_RIGHT;
			case DOWN_2:
				return wall_LEFT;
			case UP_5:
				return wall_RIGHT;
			default:
				return wall_RIGHT;
		}
	}
}

int upOrDown(int r, int c){ // 0 = up; 1 = down
	return abs( (r % 2) - (c % 2) );
}

void print_path(int r, int c){
	r++;
	c++;
	printf("%d,%d\n", r, c);
}

enum test_e validateMaze(Map *maze){
	for (int row = 0; row < maze -> rows; row++){
		for (int col = 0; col < maze -> cols; col++){
			// check left
			if (col == 0){
				continue;
			}
			if (isborder(maze, row, col - 1, wall_RIGHT) != isborder(maze, row, col, wall_LEFT)){
				return test_INVALID;
			}
			//check up / down
			
			if (row % 2 == 0){
				if (col % 2 == 0){
					if (row == 0){
						continue;
					}
					if (isborder(maze, row - 1, col, wall_UP_DOWN) != isborder(maze, row, col, wall_UP_DOWN)){
						return test_INVALID;
					}
				} else /* col % 2 != 0 */ {
					if (row == maze -> rows - 1){
						continue;
					}
					if (isborder(maze, row, col, wall_UP_DOWN) != isborder(maze, row + 1, col, wall_UP_DOWN)){
						return test_INVALID;
					}
				}
			} else /* row % 2 != 0 */{
				if (col % 2 == 0){
					if (row == maze -> rows - 1){
						continue;
					}
					if (isborder(maze, row, col, wall_UP_DOWN) != isborder(maze, row + 1, col, wall_UP_DOWN)){
						return test_INVALID;
					}
				} else  /* col % 2 != 0 */ {
					if (row == 0){
						continue;
					}
					if (isborder(maze, row - 1, col, wall_UP_DOWN) != isborder(maze, row, col, wall_UP_DOWN)){
						return test_INVALID;
					}
				}
			}
		}
	}
	return test_VALID;
}
bool isborder(Map *map, int r, int c, int border){
	int num = map -> cells[c + map->cols * r];
	return checkBit(num, border);
}

bool consumeStream(FILE *fptr, int* num, bool multiple){
	char c;
	if (multiple == false) {
		c = (char) fgetc(fptr);
		if (c == EOF) {
			return false;
		}
		if (c < 48 || c > 55) { // TODO: Does this ever trigger???????
			return false;
		}
		
		*num = c - 48;
	}else{
		char buf[10] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
		int ind=0;
		while (true){
			c = (char) fgetc(fptr);
			if (c >= '0' && c <= '9'){
				buf[ind] = c;
				ind++;
			}else{
				break;
			}
		}
		if (strlen(buf) == 0){
			return false;
		}
		*num = atoi(buf);
	}
	while (true){
		c = (char) fgetc(fptr);
		if (c == EOF){
			return true;
		}
		if  (c != ' ' && c != '\f' && c != '\n' && c != '\r' && c != '\t' && c != '\v'){
			fseek(fptr, -1L, SEEK_CUR);
			return true;
		}
	}
}

enum test_e initMaze(Map *maze, file_t *fileArg){
	FILE *fptr;
	fptr = fopen(fileArg->name, "r");
	if (fptr == NULL){
		return test_FILE;
	}
	int num = 0;
	
	
	bool res;
	char c;
	while (true){
		c = (char) fgetc(fptr);
		if  (c != ' ' && c != '\f' && c != '\n' && c != '\r' && c != '\t' && c != '\v'){
			fseek(fptr, -1L, SEEK_CUR);
			break;
		}
	}
	
	if ((res = consumeStream(fptr, &num, true)) == false){
		return test_INVALID;
	}
	maze -> rows = num;
	if ((res = consumeStream(fptr, &num, true)) == false){
		return test_INVALID;
	}
	maze -> cols = num;
	if ((maze -> cells = malloc(sizeof(int) * maze -> rows * maze -> cols)) == NULL){
		return test_MALLOC;
	}
	
	int cnt=0;
	
	while ((res = consumeStream(fptr, &num, false)) == true && cnt < maze -> cols * maze -> rows){
		maze -> cells[cnt] = num;
		cnt++;
	}
	
	if (cnt != maze -> cols * maze -> rows){
		return test_INVALID;
	}
	
	fclose(fptr);
	return test_VALID;
}

enum mode handleArg(int argc, char**argv, file_t *file){
	if (argc == 0){
		return INVALID;
	}
	if(strcmp(argv[1], "--help") == 0){
		return HELP;
	}
	if (argc < 3){
		return INVALID;
	}
	
	if (strcmp(argv[1], "--test") == 0){
		if ((file->name = malloc(sizeof(char) * strlen(argv[2]))) == NULL){
			return INVALID;
		}
		strcpy(file->name, argv[2]);
		file->len = (int) strlen(file->name);
		return TEST;
	}
	if ((file->name = malloc(sizeof(char) * strlen(argv[4]))) == NULL){
		return INVALID;
	}
	if (argc < 5 ){
		return INVALID;
	}
	strcpy(file->name, argv[4]);
	file->len = (int) strlen(file->name);
	file -> r = atoi(argv[2]) - 1;
	file -> c = atoi(argv[3]) - 1;
	
	if(strcmp(argv[1], "--rpath") == 0){
		return RPATH;
	} else if(strcmp(argv[1], "--lpath") == 0){
		return LPATH;
	} else if(strcmp(argv[1], "--shortest") == 0){
		return SHORTEST;
	}
	return INVALID;
}

void destroy(void* ptr){
	if (ptr != NULL){
		free(ptr);
	}
}

bool checkBit( int num, int pos){
	int mask = poww(2, pos);
	return (num & mask) == mask;
}

int poww(int num, int exp){
	int res = 1;
	for (int i = 0; i < exp; i++){
		res *= num;
	}
	return res;
}