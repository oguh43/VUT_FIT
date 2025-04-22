// proj2.c
// Řešení IOS-DU2, 28.4.2024
// Autor: Hugo Bohácsek (xbohach00), FIT
// Přeloženo: gcc (GCC) 13.2.1 20240417

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define L_MAX 20000			// Maximum skiers.
#define Z_MAX 10			// Maximum stops.
#define K_MAX 100			// Maximum bus capacity.
#define TL_MAX 10000		// Maximum time to generate new skier.
#define TB_MAX 1000			// Maximum time of bus driving.
#define ARG_COUNT 5			// Number of arguments.

// Semaphores names.
#define SEM_FINAL 		"/xbohach00-final"
#define SEM_MUTEX 		"/xbohach00-mutex"
#define SEM_WAITING 	"/xbohach00-waiting"
#define SEM_WRITING 	"/xbohach00-writing"
#define SEM_BOARDING 	"/xbohach00-boarding"

typedef struct {
	short int L;	// Ammount of skiers
	short int Z;	// Ammount of stops
	short int K;	// Capacity of the bus.
	short int TL;	// Maximum time to generate new skier.
	short int TB;	// Maximum time of bus driving.
}args_t;

typedef struct {
    unsigned int 	line_number;				// Number of lines in the output file.
    short int 		skiers_boarded;				// How many skiers are currently on the bus.
    short int 		skiers_waiting;				// How many skiers are waiting for the bus.
    short int 		skiers_per_stop[Z_MAX + 1];	// How many skiers are waiting at each stop.
} SharedData;

void		semaphores_destroy(sem_t *sem_list[]);
SharedData	*create_shared_memory(void);
void 		error_exit(const char *fmt, ...);
args_t 		*get_args(int argc, char *argv[]);
void 		print_line(const char * fmt, ...);
void		skibus_process(sem_t *sem_list[]);
void		semaphores_init(sem_t *sem_list[10]);
short int 	random_number(short int max, short int seed);
void		skier_process(int skier_id, sem_t *sem_list[]);

// Semaphores.
sem_t *sem_final;
sem_t *sem_mutex;
sem_t *sem_waiting;
sem_t *sem_writing;
sem_t *sem_boarding;

// Other variables.
FILE		*file;
args_t		*args = NULL;
SharedData	*data = NULL;

args_t *get_args(int argc, char *argv[]) {
	// Parse arguments.
	if (argc != ARG_COUNT + 1) {
		error_exit("Invalid number of arguments");
	}
	args_t *args = malloc(sizeof(args_t));
	if (args == NULL){
		error_exit("Failed to allocate memory for args");
	}
	args -> L = atoi(argv[1]);
	args -> Z = atoi(argv[2]);
	args -> K = atoi(argv[3]);
	args -> TL = atoi(argv[4]);
	args -> TB = atoi(argv[5]);
	if (args -> L < 1 || args -> L >= L_MAX || args -> Z < 1 || args -> Z > Z_MAX || args -> K < 10 || args -> K > K_MAX || args -> TL < 0 || args -> TL > TL_MAX || args -> TB < 0 || args -> TB > TB_MAX) {
		free(args);
		error_exit("Invalid arguments");
	}
	return args;
}

SharedData* create_shared_memory(void) {
	// Create shared memory for data.
    return mmap(NULL, sizeof(unsigned int) + sizeof(short int) * (Z_MAX + 2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

void error_exit(const char *fmt, ...){ // Prevziate z môjho riešenia tohtoročného ijc projektu.
	// Print error message and exit.
    fprintf(stderr, "Error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

short int random_number(short int max, short int seed){
	// Generate a random number between 0 and max.
	srand(time(0) + seed);
	return (rand() % max);
}

void print_line(const char * fmt, ...) {
    // We need to acquire lock so we can safely write to file and increment shared line number.
    sem_wait(sem_writing);
    va_list args;
    va_start(args, fmt);
	fprintf(file, "%d: ", ++data -> line_number);
    vfprintf(file, fmt, args);
    fprintf(file, "\n");
    fflush(file);
    va_end(args);
	sem_post(sem_writing);
}

void semaphores_init(sem_t *sem_list[10]) {
	// Unlink all semaphores in case they were left open by past run (probably due to a crash).
	sem_unlink(SEM_FINAL);
	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_WAITING);
	sem_unlink(SEM_WRITING);
	sem_unlink(SEM_BOARDING);

	for (int i = 0; i < 11; i++) {
		char SEM_NAME[15];
		sprintf(SEM_NAME, "/xbohach00-%d", i);
		sem_unlink(SEM_NAME);
	}

	// Open fresh semaphores.
    sem_final 		= sem_open(SEM_FINAL, 		O_CREAT | O_EXCL, 0666, 0);
	sem_mutex 		= sem_open(SEM_MUTEX, 		O_CREAT | O_EXCL, 0666, 1);
    sem_waiting 	= sem_open(SEM_WAITING, 	O_CREAT | O_EXCL, 0666, 0);
    sem_writing 	= sem_open(SEM_WRITING, 	O_CREAT | O_EXCL, 0666, 1);
    sem_boarding 	= sem_open(SEM_BOARDING, 	O_CREAT | O_EXCL, 0666, 0);

	if (sem_writing == SEM_FAILED || sem_waiting == SEM_FAILED || sem_boarding == SEM_FAILED || sem_final == SEM_FAILED || sem_mutex == SEM_FAILED) {
		error_exit("Failed to create semaphores");
	}

	// Open semaphores for each bus stop.
	for (int i = 0; i < args -> Z; i++) {
		char SEM_NAME[20];
		sprintf(SEM_NAME, "/xbohach00-%d", i);
		sem_list[i] = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
		if (sem_list[i] == SEM_FAILED) {
			error_exit("Failed to create semaphore %d", i);
		}
	}
}

void semaphores_destroy(sem_t *sem_list[]) {
	// Destroy all semaphores.
    if (sem_close(sem_writing) || sem_close(sem_waiting) || sem_close(sem_boarding) || sem_close(sem_final) || sem_close(sem_mutex) || sem_unlink(SEM_WRITING) || sem_unlink(SEM_WAITING) || sem_unlink(SEM_BOARDING) || sem_unlink(SEM_FINAL) || sem_unlink(SEM_MUTEX)){
		error_exit("Failed to unlink/ close semaphores!");
	}
	for (int i = 0; i < args -> Z; i++) {
		char SEM_NAME[20];
		sprintf(SEM_NAME, "/xbohach00-%d", i);
		if (sem_unlink(SEM_NAME)) {
			error_exit("Failed to unlink semaphore %d", i);
		}
		if (sem_close(sem_list[i])) {
			error_exit("Failed to close semaphore %d", i);
		}
	}
	if (munmap(data, sizeof(SharedData)) == -1) {
		error_exit("Failed to delete shared memory");
	}
	if (fclose(file) == EOF) {
		error_exit("Failed to close file");
	}
	if (args != NULL){
		free(args);
	}
}

void skibus_process(sem_t *sem_list[]) {
	// Skibus process.
    print_line("BUS: started");

    int current_stop = 0;

    for(;;){
        // Traveling to the next stop.
        usleep(random_number(args->TB,current_stop));
        print_line("BUS: arrived to %d", current_stop + 1);

        // Skiers boarding.
        for(;;){
            sem_wait(sem_mutex);
            if (data->skiers_per_stop[current_stop]==0||data->skiers_boarded== args->K){
                sem_post(sem_mutex);
                break;
            } else {
				sem_post(sem_mutex);
				sem_post(sem_list[current_stop]); 	// One skier can board.
				sem_wait(sem_boarding); 			// We wait for the skier to board.
            }        
        }

		// Leaving the stop.
        print_line("BUS: leaving %d", current_stop+1);
        if (current_stop < args->Z-1) {
            current_stop++;
        } else {
			// Final stop.
            print_line("BUS: arrived to final");
            while (1)
            {
                sem_wait(sem_mutex); 				// We need to access shared memory, thus we wait for mutex.
                if (data->skiers_boarded == 0){
                    sem_post(sem_mutex);
                    break;
                }
                sem_post(sem_mutex);
		        sem_post(sem_final); 			// One skier gets off.
		        sem_wait(sem_boarding); 			// Wait for the skier to get off.
            }
            print_line("BUS: leaving final");
            

            sem_wait(sem_mutex); 					// We need to access shared memory, thus we wait for mutex.
            if (data->skiers_waiting == 0) { 		// Are we done?
                print_line("BUS: finish");
                sem_post(sem_mutex);
                break;
            }
            sem_post(sem_mutex);

            current_stop = 0;
        }
    }
    exit(EXIT_SUCCESS);
}


void skier_process(int skier_id, sem_t *sem_list[]) {
	// Skier process.
    print_line("L %d: started", skier_id+1);

    // Walking to the stop.
    int stop = random_number(args -> Z, skier_id+1);
    usleep(random_number(args -> TL,skier_id+1));

    sem_wait(sem_mutex); 		// We need to access shared memory, thus we wait for mutex.
	print_line("L %d: arrived to %d", skier_id+1, stop+1);
    data->skiers_per_stop[stop]++;
	sem_post(sem_mutex);

    for(;;){
        // Waiting for the bus.
        sem_wait(sem_list[stop]);
    
        sem_wait(sem_mutex); 	// We need to access shared memory, thus we wait for mutex.
        if(data->skiers_boarded < args -> K) {
            data->skiers_waiting--;
            data->skiers_per_stop[stop]--;
            data->skiers_boarded++;
			print_line("L %d: boarding", skier_id+1);
            sem_post(sem_mutex);
            sem_post(sem_boarding);
            break;
        }
        sem_post(sem_mutex);
        sem_post(sem_boarding); // Signal we got on.
    }
    // Wait for the final stop
    sem_wait(sem_final);

    sem_wait(sem_mutex); 		// We need to access shared memory, thus we wait for mutex.
    data->skiers_boarded--;
	print_line("L %d: going to ski", skier_id+1);

    sem_post(sem_mutex);

    sem_post(sem_boarding); 	// Signal we got off.

    exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) {
	// Main function.
	srand(time(NULL)); // Initialize random seed.

    // Parse arguments.
	args = get_args(argc, argv);

    // Initialize shared memory.
    data = create_shared_memory(); 
	data->skiers_waiting = args->L;

	// Initialize semaphores.
	sem_t *sem_list[10];
	semaphores_init(sem_list);

	// Open output file.
    if((file = fopen("proj2.out", "w+")) == NULL) {
        error_exit("Failed to open or create output file");
    }

    // Process creation.
    // Try to fork skibus.
    pid_t skibus_pid = fork();
    if (skibus_pid == -1) {
		semaphores_destroy(sem_list);
        error_exit("Failed to fork skibus!");
    } else if (skibus_pid == 0) {
        skibus_process(sem_list);
        exit(EXIT_SUCCESS);
    }

    // Try to fork skiers.
    for (int i = 0; i < args -> L; i++) {
        pid_t skier_pid = fork();
        if (skier_pid == -1) {
			semaphores_destroy(sem_list);
            error_exit("Failed to fork skier!");
        } else if (skier_pid == 0) {
            skier_process(i, sem_list);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all children to finish.
    for (int i = 0; i < args -> L + 1; i++) {
        if (wait(NULL) == -1) {
            semaphores_destroy(sem_list);
            error_exit("Failed to wait for child process!");
        }
    }

    // Clean up.

    semaphores_destroy(sem_list);

	return EXIT_SUCCESS;
}
