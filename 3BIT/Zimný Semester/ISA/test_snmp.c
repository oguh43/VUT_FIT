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
*       Implementation of the Unit tests for SNMP protocol functionality       *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"
#include <assert.h>

/* External test framework functions from test_oid.c */
extern int test_count;
extern int test_passed;

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
			printf("FAIL: %s (expected %d, got %d)\n", msg, (int) expected, actual); \
			return; \
		} \
	} while(0)

void test_encode_string() {
	TEST_START("encode_string");
	
	unsigned char buffer[100];
	const char *test_string = "public";
	int encoded_len = encode_string(buffer, test_string, (int) strlen(test_string));
	
	ASSERT_TRUE(encoded_len > 0, "String encoding should succeed");
	ASSERT_EQUAL(ASN1_OCTET_STRING, buffer[0], "First byte should be OCTET_STRING tag");
	ASSERT_EQUAL(strlen(test_string), buffer[1], "Length should match string length");
	
	/* Check that the string data follows */
	for (int i = 0; i < (int)strlen(test_string); i++) {
		ASSERT_EQUAL(test_string[i], buffer[2 + i], "String content should match");
	}
	
	TEST_PASS();
}

void test_encode_oid_basic() {
	TEST_START("encode_oid_basic");
	
	oid_t oid;
	unsigned char buffer[100];
	
	/* Parse a simple OID */
	int parse_result = parse_oid_string("1.3.6.1.2.1.1.1.0", &oid);
	ASSERT_EQUAL(0, parse_result, "OID parsing should succeed");
	
	int encoded_len = encode_oid(buffer, &oid);
	ASSERT_TRUE(encoded_len > 0, "OID encoding should succeed");
	ASSERT_EQUAL(ASN1_OBJECT_ID, buffer[0], "First byte should be OBJECT_ID tag");
	
	/* Sanity check */
	ASSERT_TRUE(encoded_len >= 3 && encoded_len <= 50, "Encoded OID length should be reasonable");
	
	TEST_PASS();
}

void test_build_snmp_get_request() {
	TEST_START("build_snmp_get_request");
	
	oid_list_t oids;
	unsigned char buffer[1000];
	
	/* Create a simple OID list */
	oids.count = 1;
	parse_oid_string("1.3.6.1.2.1.1.1.0", &oids.oids[0]);
	
	int request_len = build_snmp_get_request(buffer, "public", &oids, 12345);
	
	ASSERT_TRUE(request_len > 0, "SNMP request building should succeed");
	ASSERT_EQUAL(ASN1_SEQUENCE, buffer[0], "SNMP message should start with SEQUENCE");
	
	/* Sanity checks */
	ASSERT_TRUE(request_len >= 20 && request_len <= 1000, "Request length should be reasonable");
	
	TEST_PASS();
}

void test_snmp_request_structure() {
	TEST_START("snmp_request_structure");
	
	oid_list_t oids;
	unsigned char buffer[1000];
	
	/* Create multiple OIDs */
	oids.count = 3;
	parse_oid_string("1.3.6.1.2.1.1.1.0", &oids.oids[0]);
	parse_oid_string("1.3.6.1.2.1.1.3.0", &oids.oids[1]);
	parse_oid_string("1.3.6.1.2.1.1.5.0", &oids.oids[2]);
	
	int request_len = build_snmp_get_request(buffer, "test_community", &oids, 67890);
	
	ASSERT_TRUE(request_len > 0, "Multi-OID SNMP request should succeed");
	
	/* The request should be larger for multiple OIDs */
	ASSERT_TRUE(request_len >= 50, "Multi-OID request should be reasonably sized");
	
	TEST_PASS();
}

void test_parse_url_http() {
	TEST_START("parse_url_http");
	
	char host[MAX_HOSTNAME_LEN];
	char path[MAX_URL_LEN];
	int port, use_https;
	
	int result = parse_url("http://localhost:4318/v1/metrics", host, &port, path, &use_https);
	
	ASSERT_EQUAL(0, result, "URL parsing should succeed");
	ASSERT_TRUE(strcmp(host, "localhost") == 0, "Host should be 'localhost'");
	ASSERT_EQUAL(4318, port, "Port should be 4318");
	ASSERT_TRUE(strcmp(path, "/v1/metrics") == 0, "Path should be '/v1/metrics'");
	ASSERT_EQUAL(0, use_https, "Should not use HTTPS");
	
	TEST_PASS();
}

void test_parse_url_https() {
	TEST_START("parse_url_https");
	
	char host[MAX_HOSTNAME_LEN];
	char path[MAX_URL_LEN];
	int port, use_https;
	
	int result = parse_url("https://otel.example.com/api/v1/metrics", host, &port, path, &use_https);
	
	ASSERT_EQUAL(0, result, "HTTPS URL parsing should succeed");
	ASSERT_TRUE(strcmp(host, "otel.example.com") == 0, "Host should be correct");
	ASSERT_EQUAL(443, port, "Default HTTPS port should be 443");
	ASSERT_TRUE(strcmp(path, "/api/v1/metrics") == 0, "Path should be correct");
	ASSERT_EQUAL(1, use_https, "Should use HTTPS");
	
	TEST_PASS();
}

void test_parse_url_with_port() {
	TEST_START("parse_url_with_port");
	
	char host[MAX_HOSTNAME_LEN];
	char path[MAX_URL_LEN];
	int port, use_https;
	
	int result = parse_url("http://192.168.1.100:8080/metrics", host, &port, path, &use_https);
	
	ASSERT_EQUAL(0, result, "URL with custom port should parse");
	ASSERT_TRUE(strcmp(host, "192.168.1.100") == 0, "IP address host should be correct");
	ASSERT_EQUAL(8080, port, "Custom port should be parsed");
	ASSERT_TRUE(strcmp(path, "/metrics") == 0, "Path should be correct");
	
	TEST_PASS();
}

void test_parse_url_invalid() {
	TEST_START("parse_url_invalid");
	
	char host[MAX_HOSTNAME_LEN];
	char path[MAX_URL_LEN];
	int port, use_https;
	
	/* Test invalid scheme */
	int result = parse_url("ftp://example.com/path", host, &port, path, &use_https);
	ASSERT_TRUE(result != 0, "Invalid scheme should fail");
	
	/* Test malformed URL */
	result = parse_url("not_a_url", host, &port, path, &use_https);
	ASSERT_TRUE(result != 0, "Malformed URL should fail");
	
	TEST_PASS();
}

void test_json_escape_string() {
	TEST_START("json_escape_string");
	
	char output[256];
	
	/* Test basic string */
	json_escape_string("simple_string", output, sizeof(output));
	ASSERT_TRUE(strcmp(output, "simple_string") == 0, "Simple string should not be modified");
	
	/* Test string with quotes */
	json_escape_string("string with \"quotes\"", output, sizeof(output));
	ASSERT_TRUE(strstr(output, "\\\"") != NULL, "Quotes should be escaped");
	
	/* Test string with backslashes */
	json_escape_string("path\\to\\file", output, sizeof(output));
	ASSERT_TRUE(strstr(output, "\\\\") != NULL, "Backslashes should be escaped");
	
	/* Test string with newlines */
	json_escape_string("line1\nline2", output, sizeof(output));
	ASSERT_TRUE(strstr(output, "\\n") != NULL, "Newlines should be escaped");
	
	TEST_PASS();
}

void test_generate_metric_name() {
	TEST_START("generate_metric_name");
	
	oid_t oid;
	char name[256];
	
	parse_oid_string("1.3.6.1.2.1.1.3.0", &oid);
	generate_metric_name_from_oid(&oid, name, sizeof(name));
	
	ASSERT_TRUE(strstr(name, "snmp_oid_") != NULL, "Metric name should have snmp.oid prefix");
	ASSERT_TRUE(strstr(name, "1_3_6_1_2_1_1_3_0") != NULL, "OID should be in metric name with underscores");
	
	TEST_PASS();
}

void test_snmp_value_to_string() {
	TEST_START("snmp_value_to_string");
	
	snmp_var_t var;
	char value_str[64];
	
	/* Test INTEGER value */
	var.type = ASN1_INTEGER;
	var.value.integer_val = 12345;
	snmp_value_to_string(&var, value_str, sizeof(value_str));
	ASSERT_TRUE(strcmp(value_str, "12345") == 0, "Integer should convert to string correctly");
	
	/* Test OCTET STRING with numeric content */
	var.type = ASN1_OCTET_STRING;
	strcpy(var.value.string_val, "67890");
	snmp_value_to_string(&var, value_str, sizeof(value_str));
	ASSERT_TRUE(strcmp(value_str, "67890") == 0, "Numeric string should convert correctly");
	
	/* Test OCTET STRING with non-numeric content */
	strcpy(var.value.string_val, "hello_world");
	snmp_value_to_string(&var, value_str, sizeof(value_str));
	ASSERT_TRUE(atoi(value_str) == 11, "Non-numeric string should use length as value");
	
	TEST_PASS();
}

/* Run SNMP-specific tests */
void run_snmp_tests() {
	printf("\nRunning SNMP protocol tests...\n");
	
	test_encode_string();
	test_encode_oid_basic();
	test_build_snmp_get_request();
	test_snmp_request_structure();
	
	printf("\nRunning HTTP and URL parsing tests...\n");
	
	test_parse_url_http();
	test_parse_url_https();
	test_parse_url_with_port();
	test_parse_url_invalid();
	
	printf("\nRunning OpenTelemetry formatting tests...\n");
	
	test_json_escape_string();
	test_generate_metric_name();
	test_snmp_value_to_string();
}