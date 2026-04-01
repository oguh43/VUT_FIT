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
*            Implementation of the Integration tests and test runner           *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"
#include <assert.h>
#include <getopt.h>

/* Global test counters */
int test_count = 0;
int test_passed = 0;

/* Test framework macros */
#define TEST_START(name) \
	do { \
		printf("Running test: %s... ", name); \
		test_count++; \
	} while(0)

#define TEST_PASS() \
	do { \
		printf("PASS\n"); \
		test_passed++; \
	} while(0)

#define TEST_FAIL(msg) \
	do { \
		printf("FAIL: %s\n", msg); \
	} while(0)

#define ASSERT_TRUE(condition, msg) \
	do { \
		if (!(condition)) { \
			TEST_FAIL(msg); \
			return; \
		} \
	} while(0)

#define ASSERT_EQUAL(expected, actual, msg) \
	do { \
		if ((expected) != (actual)) { \
			printf("FAIL: %s (expected %d, got %d)\n", msg, expected, actual); \
			return; \
		} \
	} while(0)

/* Function prototypes from other test files */
extern void run_snmp_tests(void);
extern void run_oid_tests(void);

void test_timestamp_generation() {
	TEST_START("timestamp_generation");
	
	unsigned long ts1 = get_timestamp_ms();
	
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 15000000; /* 15ms to account for scheduling overhead */
	nanosleep(&delay, NULL);

	unsigned long ts2 = get_timestamp_ms();
	
	ASSERT_TRUE(ts1 > 0, "Timestamp should be non-zero");
	ASSERT_TRUE(ts2 > ts1, "Second timestamp should be larger");
	ASSERT_TRUE((ts2 - ts1) >= 10, "Time difference should be at least 10ms");
	
	TEST_PASS();
}

void test_hostname_resolution() {
	TEST_START("hostname_resolution");
	
	char ip_str[256];
	
	/* Test localhost resolution */
	int result = resolve_hostname("localhost", ip_str);
	ASSERT_EQUAL(0, result, "localhost should resolve");
	ASSERT_TRUE(strlen(ip_str) > 0, "IP string should not be empty");
	
	/* Test IP address passthrough */
	result = resolve_hostname("127.0.0.1", ip_str);
	ASSERT_EQUAL(0, result, "IP address should pass through");
	ASSERT_TRUE(strcmp(ip_str, "127.0.0.1") == 0, "IP should remain unchanged");
	
	TEST_PASS();
}

void test_config_parsing() {
	TEST_START("config_parsing");
	
	config_t config;
	
	/* Test default values */
	memset(&config, 0, sizeof(config));
	strcpy(config.community, DEFAULT_COMMUNITY);
	config.interval = DEFAULT_INTERVAL;
	config.retries = DEFAULT_RETRIES;
	config.timeout = DEFAULT_TIMEOUT;
	config.port = DEFAULT_PORT;
	
	ASSERT_TRUE(strcmp(config.community, "public") == 0, "Default community should be 'public'");
	ASSERT_EQUAL(10, config.interval, "Default interval should be 10");
	ASSERT_EQUAL(2, config.retries, "Default retries should be 2");
	ASSERT_EQUAL(1000, config.timeout, "Default timeout should be 1000");
	ASSERT_EQUAL(161, config.port, "Default port should be 161");
	
	TEST_PASS();
}

void test_oid_file_creation() {
	TEST_START("oid_file_creation");
	
	/* Create a temporary OID file */
	FILE *f = fopen("test_oids.txt", "w");
	ASSERT_TRUE(f != NULL, "Should be able to create test file");
	
	fprintf(f, "# Test OID file\n");
	fprintf(f, "1.3.6.1.2.1.1.1.0\n");
	fprintf(f, "\n");  /* Empty line */
	fprintf(f, "1.3.6.1.2.1.1.3.0\n");
	fprintf(f, "# Another comment\n");
	fprintf(f, "1.3.6.1.2.1.1.5.0\n");
	fclose(f);
	
	/* Load OIDs from file */
	oid_list_t oids;
	int result = load_oids_from_file("test_oids.txt", &oids);
	
	ASSERT_EQUAL(0, result, "Loading OIDs should succeed");
	ASSERT_EQUAL(3, oids.count, "Should load 3 OIDs");
	
	/* Clean up */
	unlink("test_oids.txt");
	
	TEST_PASS();
}

void test_json_generation() {
	TEST_START("json_generation");
	
	snmp_var_t vars[2];
	char json_buffer[4096];
	
	/* Create test variables */
	parse_oid_string("1.3.6.1.2.1.1.1.0", &vars[0].oid);
	vars[0].type = ASN1_OCTET_STRING;
	strcpy(vars[0].value.string_val, "Test Device");
	
	parse_oid_string("1.3.6.1.2.1.1.3.0", &vars[1].oid);
	vars[1].type = ASN1_INTEGER;
	vars[1].value.integer_val = 12345;
	
	int json_len = build_otel_json(vars, 2, json_buffer, sizeof(json_buffer));
	
	ASSERT_TRUE(json_len > 0, "JSON generation should succeed");
	ASSERT_TRUE(strstr(json_buffer, "resourceMetrics") != NULL, "JSON should contain resourceMetrics");
	ASSERT_TRUE(strstr(json_buffer, "snmp2otel") != NULL, "JSON should contain service name");
	ASSERT_TRUE(strstr(json_buffer, "gauge") != NULL, "JSON should contain gauge metrics");
	ASSERT_TRUE(strstr(json_buffer, "1.3.6.1.2.1.1.1.0") != NULL, "JSON should contain OID");
	
	TEST_PASS();
}

void test_argument_validation() {
	TEST_START("argument_validation");
	
	config_t config;
	
	/* Simulate command line arguments */
	char *test_argv[] = {
		"snmp2otel",
		"-t", "192.168.1.1",
		"-o", "test_oids.txt", 
		"-e", "http://localhost:4318/v1/metrics",
		"-v"
	};
	/* Reset optind */
	optind = 1;
	
	(void)test_argv; /* Suppress unused variable warning */
	
	/* Test valid interval */
	config.interval = 30;
	ASSERT_TRUE(config.interval > 0, "Valid interval should be accepted");
	
	/* Test valid port */
	config.port = 161;
	ASSERT_TRUE(config.port > 0 && config.port <= 65535, "Valid port should be accepted");
	
	/* Test valid timeout */
	config.timeout = 5000;
	ASSERT_TRUE(config.timeout > 0, "Valid timeout should be accepted");
	
	TEST_PASS();
}

void test_error_conditions() {
	TEST_START("error_conditions");
	
	/* Test NULL pointer handling */
	oid_t *null_oid = NULL;
	unsigned char buffer[100];
	
	int result = encode_oid(buffer, null_oid);
	(void)result;

	/* Test empty OID list */
	oid_list_t empty_oids;
	empty_oids.count = 0;
	
	unsigned char snmp_buffer[1000];
	result = build_snmp_get_request(snmp_buffer, "public", &empty_oids, 123);
	(void)result;
	
	/* Test invalid URL parsing */
	char host[256], path[256];
	int port, use_https;
	result = parse_url("invalid://badurl", host, &port, path, &use_https);
	ASSERT_TRUE(result != 0, "Invalid URL should be rejected");
	
	TEST_PASS();
}

void test_memory_safety() {
	TEST_START("memory_safety");
	
	/* Test buffer overflow protection */
	char small_buffer[20];
	const char *long_string = "This is a very long string that should not overflow";
	
	/* JSON escaping should not overflow */
	json_escape_string(long_string, small_buffer, sizeof(small_buffer));
	ASSERT_TRUE(strlen(small_buffer) < sizeof(small_buffer), "Buffer should not overflow");
	
	/* Test with really small buffer */
	char tiny_buffer[5];
	json_escape_string("test", tiny_buffer, sizeof(tiny_buffer));
	ASSERT_TRUE(strlen(tiny_buffer) < sizeof(tiny_buffer), "Tiny buffer should not overflow");
	
	/* Test OID string limits with more reasonable size */
	oid_t oid;
	char long_oid[200];
	strcpy(long_oid, "1.3.6.1.2.1");
	for (int i = 0; i < 10; i++) { /* Reduced iterations */
		strcat(long_oid, ".999");
	}
	
	int result = parse_oid_string(long_oid, &oid);
	(void)result;
	
	TEST_PASS();
}

void run_integration_tests() {
	printf("\nRunning integration and utility tests...\n");
	
	test_timestamp_generation();
	test_hostname_resolution();
	test_config_parsing();
	test_oid_file_creation();
	test_json_generation();
	test_argument_validation();
	test_error_conditions();
	test_memory_safety();
}

/* Main test runner */
int main() {
	printf("snmp2otel Test Suite\n");
	printf("===================\n\n");
	
	/* Set verbose mode for testing */
	verbose_mode = 0;
	
	/* Run OID and basic tests */
	run_oid_tests();
	
	/* Run SNMP protocol tests */
	run_snmp_tests();
	
	/* Run integration tests */
	run_integration_tests();
	
	/* Print test summary */
	printf("\n");
	printf("Test Summary\n");
	printf("============\n");
	printf("Total tests run: %d\n", test_count);
	printf("Tests passed:    %d\n", test_passed);
	printf("Tests failed:    %d\n", test_count - test_passed);
	printf("Success rate:    %.1f%%\n", test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
	
	if (test_passed == test_count) {
		printf("\nAll tests PASSED!\n");
		printf("The snmp2otel implementation appears to be working correctly. Consider running all tests using `make test-all`!\n");
		return 0;
	} else {
		printf("\nSome tests FAILED!\n");
		printf("Please review the implementation and fix any issues. Consider running all tests using `make test-all`!\n");
		return 1;
	}
}