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
*         Implementation of the Unit tests for OID parsing functionality       *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"
#include <assert.h>

/* External test counters from test_http.c */
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
			printf("FAIL: %s (expected %d, got %d)\n", msg, expected, actual); \
			return; \
		} \
	} while(0)

#define ASSERT_STR_EQUAL(expected, actual, msg) \
	do { \
		if (strcmp(expected, actual) != 0) { \
			printf("FAIL: %s (expected '%s', got '%s')\n", msg, expected, actual); \
			return; \
		} \
	} while(0)

/* Test functions */
void test_parse_oid_valid() {
	TEST_START("parse_oid_valid");
	
	oid_t oid;
	int result = parse_oid_string("1.3.6.1.2.1.1.3.0", &oid);
	
	ASSERT_EQUAL(0, result, "parse_oid_string should succeed");
	ASSERT_EQUAL(9, oid.length, "OID should have 9 components");
	ASSERT_EQUAL(1, oid.numbers[0], "First component should be 1");
	ASSERT_EQUAL(3, oid.numbers[1], "Second component should be 3");
	ASSERT_EQUAL(0, oid.numbers[8], "Last component should be 0");
	
	TEST_PASS();
}

void test_parse_oid_with_leading_dot() {
	TEST_START("parse_oid_with_leading_dot");
	
	oid_t oid;
	int result = parse_oid_string(".1.3.6.1.2.1.1.1.0", &oid);
	
	ASSERT_EQUAL(0, result, "parse_oid_string should succeed with leading dot");
	ASSERT_EQUAL(9, oid.length, "OID should have 9 components");
	ASSERT_EQUAL(1, oid.numbers[0], "First component should be 1");
	
	TEST_PASS();
}

void test_parse_oid_invalid_empty() {
	TEST_START("parse_oid_invalid_empty");
	
	oid_t oid;
	int result = parse_oid_string("", &oid);
	
	ASSERT_TRUE(result != 0, "Empty OID should fail");
	
	TEST_PASS();
}

void test_parse_oid_invalid_non_numeric() {
	TEST_START("parse_oid_invalid_non_numeric");
	
	oid_t oid;
	int result = parse_oid_string("1.3.abc.1", &oid);
	
	ASSERT_TRUE(result != 0, "Non-numeric OID component should fail");
	
	TEST_PASS();
}

void test_compare_oids_equal() {
	TEST_START("compare_oids_equal");
	
	oid_t oid1, oid2;
	parse_oid_string("1.3.6.1.2.1.1.3.0", &oid1);
	parse_oid_string("1.3.6.1.2.1.1.3.0", &oid2);
	
	int result = compare_oids(&oid1, &oid2);
	ASSERT_EQUAL(0, result, "Identical OIDs should compare equal");
	
	TEST_PASS();
}

void test_compare_oids_different() {
	TEST_START("compare_oids_different");
	
	oid_t oid1, oid2;
	parse_oid_string("1.3.6.1.2.1.1.3.0", &oid1);
	parse_oid_string("1.3.6.1.2.1.1.1.0", &oid2);
	
	int result = compare_oids(&oid1, &oid2);
	ASSERT_TRUE(result != 0, "Different OIDs should not compare equal");
	
	TEST_PASS();
}

void test_encode_decode_length() {
	TEST_START("encode_decode_length");
	
	unsigned char buffer[10];
	int encoded_len, decoded_len, bytes_read;
	
	/* Test short form */
	encoded_len = encode_length(buffer, 42);
	ASSERT_EQUAL(1, encoded_len, "Short form should use 1 byte");
	ASSERT_EQUAL(42, buffer[0], "Short form encoding incorrect");
	
	decode_length(buffer, &decoded_len, &bytes_read);
	ASSERT_EQUAL(42, decoded_len, "Short form decoding incorrect");
	ASSERT_EQUAL(1, bytes_read, "Short form should read 1 byte");
	
	/* Test long form */
	encoded_len = encode_length(buffer, 300);
	ASSERT_TRUE(encoded_len > 1, "Long form should use more than 1 byte");
	
	decode_length(buffer, &decoded_len, &bytes_read);
	ASSERT_EQUAL(300, decoded_len, "Long form decoding incorrect");
	
	TEST_PASS();
}

void test_encode_decode_integer() {
	TEST_START("encode_decode_integer");
	
	unsigned char buffer[10];
	int encoded_len, decoded_val;
	
	/* Test positive integer */
	encoded_len = encode_integer(buffer, 12345);
	ASSERT_TRUE(encoded_len > 0, "Integer encoding should succeed");
	ASSERT_EQUAL(ASN1_INTEGER, buffer[0], "First byte should be INTEGER tag");
	
	/* Decode the integer */
	int length, bytes_read;
	decode_length(&buffer[1], &length, &bytes_read);
	decode_integer(&buffer[1 + bytes_read], length, &decoded_val);
	ASSERT_EQUAL(12345, decoded_val, "Integer decoding incorrect");
	
	/* Test zero */
	encoded_len = encode_integer(buffer, 0);
	decode_length(&buffer[1], &length, &bytes_read);
	decode_integer(&buffer[1 + bytes_read], length, &decoded_val);
	ASSERT_EQUAL(0, decoded_val, "Zero encoding/decoding incorrect");
	
	TEST_PASS();
}

/* Run basic OID and ASN.1 tests */
void run_oid_tests() {
	printf("Running basic OID parsing tests...\n");
	
	/* OID tests */
	test_parse_oid_valid();
	test_parse_oid_with_leading_dot();
	test_parse_oid_invalid_empty();
	test_parse_oid_invalid_non_numeric();
	test_compare_oids_equal();
	test_compare_oids_different();
	
	/* ASN.1 BER tests */
	test_encode_decode_length();
	test_encode_decode_integer();
}