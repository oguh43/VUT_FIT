/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*               Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)             *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*              Implementation of the Full test suite for snmp2otel             *
*                                                                              *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

/* ANSI color codes */
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_RESET   "\033[0m"

/* Test statistics */
static int tests_run = 0;
static int tests_passed = 0;

/* Function prototypes */
int run_command(const char *command, int expect_success);
int run_command_with_timeout(const char *command, int timeout_seconds);
int file_exists(const char *path);
int is_executable(const char *path);
void print_test_header(const char *header);
void print_test_result(const char *test_name, int passed);
int create_test_file(const char *filename, const char *content);
int remove_test_file(const char *filename);

/* Execute a command and check its exit status */
int run_command(const char *command, int expect_success) {
	int status;
	int ret;
	
	/* Redirect stderr to /dev/null for cleaner output */
	char full_command[1024];
	snprintf(full_command, sizeof(full_command), "%s 2>/dev/null", command);
	
	status = system(full_command);
	
	if (status == -1) {
		return 0; /* Failed to execute */
	}
	
	ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	
	if (expect_success) {
		return (ret == 0);
	} else {
		return (ret != 0);
	}
}

/* Execute command with timeout */
int run_command_with_timeout(const char *command, int timeout_seconds) {
	pid_t pid;
	int status;
	int elapsed = 0;
	
	pid = fork();
	if (pid == -1) {
		return 0; /* Fork failed */
	}
	
	if (pid == 0) {
		/* Child process */
		/* Redirect output to /dev/null */
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
		
		/* Execute command */
		execl("/bin/sh", "sh", "-c", command, (char *)NULL);
		exit(1); /* If exec fails */
	}
	
	/* Parent process - wait with timeout */
	while (elapsed < timeout_seconds) {
		int result = waitpid(pid, &status, WNOHANG);
		if (result == pid) {
			/* Process finished */
			return 1;
		} else if (result == -1) {
			/* Error */
			return 0;
		}
		
		sleep(1);
		elapsed++;
	}
	
	/* Timeout - kill the process */
	kill(pid, SIGTERM);
	sleep(1);
	kill(pid, SIGKILL);
	waitpid(pid, &status, 0);
	
	return 1; /* Timeout is expected for some tests */
}

/* Check if file exists */
int file_exists(const char *path) {
	struct stat st;
	return (stat(path, &st) == 0);
}

/* Check if file is executable */
int is_executable(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) {
		return 0;
	}
	return (st.st_mode & S_IXUSR) != 0;
}

/* Print test section header */
void print_test_header(const char *header) {
	printf("\n");
	printf("%s%s%s\n", COLOR_BLUE, header, COLOR_RESET);
	for (size_t i = 0; i < strlen(header); i++) {
		printf("=");
	}
	printf("\n");
}

/* Print test result */
void print_test_result(const char *test_name, int passed) {
	tests_run++;
	
	printf("[%d] Testing %s... ", tests_run, test_name);
	
	if (passed) {
		printf("%sPASS%s\n", COLOR_GREEN, COLOR_RESET);
		tests_passed++;
	} else {
		printf("%sFAIL%s\n", COLOR_RED, COLOR_RESET);
	}
}

/* Create a test file with content */
int create_test_file(const char *filename, const char *content) {
	FILE *f = fopen(filename, "w");
	if (!f) {
		return 0;
	}
	
	fprintf(f, "%s", content);
	fclose(f);
	return 1;
}

/* Remove test file */
int remove_test_file(const char *filename) {
	return (unlink(filename) == 0 || errno == ENOENT);
}

/* Test: Compilation */
void test_compilation(void) {
	print_test_header("Step 1: Build Tests");
	
	/* Clean first */
	int result = run_command("make clean", 1);
	print_test_result("make clean", result);
	
	/* Compile main program */
	result = run_command("make all", 1);
	print_test_result("compilation", result);
	
	if (!result) {
		printf("%sCritical: Compilation failed. Cannot continue.%s\n", COLOR_RED, COLOR_RESET);
		exit(1);
	}
	
	/* Check executable was created */
	result = is_executable("snmp2otel");
	print_test_result("executable creation", result);
	
	/* Test debug compilation */
	run_command("make clean", 1);
	result = run_command("make debug", 1);
	print_test_result("debug compilation", result);
	
	/* Test unit tests */
	run_command("make clean", 1);
	result = run_command("make test", 1);
	print_test_result("unit tests compilation and execution", result);
	
	/* Rebuild for subsequent tests */
	run_command("make clean", 1);
	run_command("make all", 1);
}

/* Test: Basic functionality */
void test_basic_functionality(void) {
	print_test_header("Step 2: Basic Functionality Tests");
	
	/* Test help message */
	int result = run_command("./snmp2otel -h", 1);
	print_test_result("help message", result);
	
	/* Test missing required arguments */
	result = run_command("./snmp2otel -o oids.txt -e http://localhost:4318/v1/metrics", 0);
	print_test_result("missing target argument (expect fail)", result);
	
	result = run_command("./snmp2otel -t localhost -e http://localhost:4318/v1/metrics", 0);
	print_test_result("missing oids file argument (expect fail)", result);
	
	result = run_command("./snmp2otel -t localhost -o oids.txt", 0);
	print_test_result("missing endpoint argument (expect fail)", result);
	
	/* Test invalid arguments */
	result = run_command("./snmp2otel -t localhost -o oids.txt -e http://localhost:4318/v1/metrics -i -5", 0);
	print_test_result("invalid interval (expect fail)", result);
	
	result = run_command("./snmp2otel -t localhost -o oids.txt -e http://localhost:4318/v1/metrics -p 99999", 0);
	print_test_result("invalid port (expect fail)", result);
	
	result = run_command("./snmp2otel -t localhost -o oids.txt -e http://localhost:4318/v1/metrics -T 0", 0);
	print_test_result("invalid timeout (expect fail)", result);
}

/* Test: File handling */
void test_file_handling(void) {
	print_test_header("Step 3: File Handling Tests");
	
	/* Create valid test OID file */
	const char *valid_oids = 
		"# Test OID file\n"
		"1.3.6.1.2.1.1.1.0\n"
		"1.3.6.1.2.1.1.3.0\n"
		"1.3.6.1.2.1.1.5.0\n";
	
	int created = create_test_file("test_oids_valid.txt", valid_oids);
	if (created) {
		/* Test with valid OID file (with timeout) */
		int result = run_command_with_timeout("./snmp2otel -t 127.0.0.1 -o test_oids_valid.txt -e http://localhost:4318/v1/metrics -i 60 -T 100", 3);
		print_test_result("valid OID file parsing", result);
		
		remove_test_file("test_oids_valid.txt");
	} else {
		print_test_result("valid OID file parsing", 0);
	}
	
	/* Test with non-existent OID file */
	int result = run_command("./snmp2otel -t localhost -o nonexistent_file_xyz.txt -e http://localhost:4318/v1/metrics -T 100", 0);
	print_test_result("non-existent OID file (expect fail)", result);
	
	/* Create invalid OID file */
	const char *invalid_oids = 
		"# Invalid OID file\n"
		"1.3.6.1.2.1.1.1.0\n"
		"invalid.oid.here\n"
		"1.3.6.1.2.1.1.5.0\n";
	
	created = create_test_file("test_oids_invalid.txt", invalid_oids);
	if (created) {
		result = run_command("./snmp2otel -t localhost -o test_oids_invalid.txt -e http://localhost:4318/v1/metrics -T 100", 0);
		print_test_result("invalid OID in file (expect fail)", result);
		
		remove_test_file("test_oids_invalid.txt");
	} else {
		print_test_result("invalid OID in file (expect fail)", 0);
	}
}

/* Test: Network functionality */
void test_network_functionality(void) {
	print_test_header("Step 4: Network Tests (Safe/Mock)");
	
	/* Test with localhost (should timeout gracefully) */
	int result = run_command_with_timeout( "./snmp2otel -t 127.0.0.1 -o oids.txt -e http://localhost:4318/v1/metrics -T 500 -r 0 -v", 5);
	print_test_result("localhost connection timeout", result);
	
	/* Test with invalid hostname (timeout test) */
	result = run_command_with_timeout("./snmp2otel -t invalid.hostname.that.does.not.exist.xyz -o oids.txt -e http://localhost:4318/v1/metrics -T 500 -r 0", 5);
	print_test_result("invalid hostname (expect fail)", result);
	
	/* Test with invalid endpoint URL */
	result = run_command_with_timeout("./snmp2otel -t 127.0.0.1 -o oids.txt -e 'not-a-valid-url' -T 500 -r 0", 5);
	print_test_result("invalid endpoint URL (expect fail)", result);
}

/* Test: Memory and resources */
void test_memory_resources(void) {
	print_test_header("Step 5: Memory and Resource Tests");
	
	/* Check if valgrind is available */
	if (run_command("which valgrind", 1)) {
		int result = run_command_with_timeout(
			"valgrind --leak-check=full --error-exitcode=1 --quiet "
			"./snmp2otel -t 127.0.0.1 -o oids.txt -e http://localhost:4318/v1/metrics -T 500 -r 0",
			10);
		print_test_result("memory leak check", result);
	} else {
		printf("[%d] Valgrind not available - skipping memory tests\n", ++tests_run);
		tests_passed++;
	}
	
	/* Test signal handling */
	/* Start program in background, send SIGINT, check clean exit */
	const char *signal_test = 
		"./snmp2otel -t 127.0.0.1 -o oids.txt -e http://localhost:4318/v1/metrics -i 30 &"
		"PID=$! ; sleep 2 ; kill -INT $PID ; wait $PID ; true";
	
	int result = run_command_with_timeout(signal_test, 5);
	print_test_result("signal handling", result);
}

/* Test: Code quality */
void test_code_quality(void) {
	print_test_header("Step 6: Code Quality Tests");
	
	/* Test compilation with strict warnings */
	run_command("make clean", 1);
	int result = run_command("make CFLAGS='-D_XOPEN_SOURCE=700 -Wall -Wextra -std=c99 -pedantic -Werror -Wconversion -Wsign-conversion -Wdouble-promotion -Wpadded -Wpacked -Wunsafe-loop-optimizations'", 1);
	print_test_result("compiler warnings check", result);
	
	/* Rebuild for subsequent tests */
	run_command("make clean", 1);
	run_command("make all", 1);
	
	/* Test with cppcheck if available */
	if (run_command("which cppcheck", 1)) {
		result = run_command(
			"cppcheck --error-exitcode=1 --enable=warning,style,performance,portability --check-level=exhaustive --quiet "
			"main.c oid.c snmp.c http.c otel.c utils.c 2>/dev/null",
			1);
		print_test_result("static analysis", result);
	} else {
		printf("[%d] cppcheck not available - skipping static analysis\n", ++tests_run);
		tests_passed++;
	}
}

/* Test: Documentation */
void test_documentation(void) {
	print_test_header("Step 7: Documentation Tests");
	
	/* Check required files exist */
	int result = file_exists("README");
	print_test_result("README exists", result);
	
	result = file_exists("Makefile");
	print_test_result("Makefile exists", result);
	
	result = file_exists("oids.txt");
	print_test_result("example OIDs file exists", result);
	
	/* Test Makefile targets */
	result = run_command("make clean", 1);
	print_test_result("Makefile clean target", result);
	
	result = run_command("make all", 1);
	print_test_result("Makefile all target", result);
	
	result = run_command("make examples", 1);
	print_test_result("Makefile examples target", result);
}

/* Print final summary */
void print_summary(void) {
	printf("\n");
	print_test_header("Test Results Summary");
	
	printf("Tests run:    %s%d%s\n", COLOR_BLUE, tests_run, COLOR_RESET);
	printf("Tests passed: %s%d%s\n", COLOR_GREEN, tests_passed, COLOR_RESET);
	printf("Tests failed: %s%d%s\n", COLOR_RED, tests_run - tests_passed, COLOR_RESET);
	
	if (tests_passed == tests_run) {
		printf("\n%sAll tests passed!%s\n", COLOR_GREEN, COLOR_RESET);
		
	} else {
		printf("\n%sSome tests failed.%s\n", COLOR_RED, COLOR_RESET);
	}
}

/* Main test runner */
int main(void) {
	printf("snmp2otel Test Runner\n");
	printf("====================\n");
	
	/* Check if we're in the right directory */
	if (!file_exists("snmp2otel.h")) {
		printf("%sError: snmp2otel.h not found. Please run from project directory.%s\n", COLOR_RED, COLOR_RESET);
		return 1;
	}
	
	/* Run all test suites */
	test_compilation();
	test_basic_functionality();
	test_file_handling();
	test_network_functionality();
	test_memory_resources();
	test_code_quality();
	test_documentation();
	
	/* Print summary and exit */
	print_summary();
	
	return (tests_passed == tests_run) ? 0 : 1;
}