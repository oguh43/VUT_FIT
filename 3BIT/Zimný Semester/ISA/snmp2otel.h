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
*             Implementation of the SNMP to OpenTelemetry exporter             *
*                                                                              *
*******************************************************************************/

#ifndef SNMP2OTEL_H
#define SNMP2OTEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

/* Configuration constants */
#define MAX_OID_LEN 128
#define MAX_OID_COUNT 1000
#define MAX_COMMUNITY_LEN 64
#define MAX_HOSTNAME_LEN 256
#define MAX_URL_LEN 512
#define MAX_LINE_LEN 256
#define MAX_BUFFER_SIZE 4096
#define MAX_JSON_SIZE 8192
#define DEFAULT_COMMUNITY "public"
#define DEFAULT_PORT 161
#define DEFAULT_INTERVAL 10
#define DEFAULT_RETRIES 2
#define DEFAULT_TIMEOUT 1000

/* SNMP constants */
#define SNMP_VERSION_2C 1
#define SNMP_GET_REQUEST 0
#define SNMP_GET_RESPONSE 1

/* ASN.1 BER tags */
#define ASN1_INTEGER 0x02
#define ASN1_OCTET_STRING 0x04
#define ASN1_NULL 0x05
#define ASN1_OBJECT_ID 0x06
#define ASN1_SEQUENCE 0x30

/* SNMP Error codes */
#define SNMP_ERR_NOERROR 0
#define SNMP_ERR_TOOBIG 1
#define SNMP_ERR_NOSUCHNAME 2
#define SNMP_ERR_BADVALUE 3
#define SNMP_ERR_READONLY 4
#define SNMP_ERR_GENERR 5

/* Structure definitions */
typedef struct {
	char target[MAX_HOSTNAME_LEN];
	char community[MAX_COMMUNITY_LEN];
	char oids_file[MAX_LINE_LEN];
	char endpoint[MAX_URL_LEN];
	int interval;
	int retries;
	int timeout;
	int port;
	int verbose;
} config_t;

typedef struct {
	unsigned int numbers[MAX_OID_LEN];
	int length;
	char str[MAX_LINE_LEN];
} oid_t;

typedef struct {
	oid_t oid;
	int type;
	union {
		int integer_val;
		char string_val[MAX_LINE_LEN];
		oid_t oid_val;
	} value;
} snmp_var_t;

typedef struct {
	oid_t oids[MAX_OID_COUNT];
	int count;
} oid_list_t;

typedef struct {
	char name[MAX_LINE_LEN];
	char unit[64];
	char type[16];
} metric_mapping_t;

/* Global variables */
extern int verbose_mode;
extern volatile int running;

/* Function prototypes */

/* main.c */
void print_usage(const char *prog_name);
int parse_arguments(int argc, char *argv[], config_t *config);
void signal_handler(int sig);

/* oid.c */
int parse_oid_string(const char *oid_str, oid_t *oid);
int load_oids_from_file(const char *filename, oid_list_t *oid_list);
void print_oid(const oid_t *oid);
int compare_oids(const oid_t *oid1, const oid_t *oid2);

/* snmp.c */
int encode_length(unsigned char *buffer, int length);
int decode_length(const unsigned char *buffer, int *length, int *bytes_read);
int encode_integer(unsigned char *buffer, int value);
int decode_integer(const unsigned char *buffer, int length, int *value);
int encode_string(unsigned char *buffer, const char *str, int len);
int decode_string(const unsigned char *buffer, int length, char *str, int max_len);
int encode_oid(unsigned char *buffer, const oid_t *oid);
int decode_oid(const unsigned char *buffer, int length, oid_t *oid);
int build_snmp_get_request(unsigned char *buffer, const char *community, const oid_list_t *oids, int request_id);
int parse_snmp_response(const unsigned char *buffer, int length, snmp_var_t *vars, int max_vars, int *var_count);
int send_snmp_request(const config_t *config, const oid_list_t *oids, snmp_var_t *results, int *result_count);

/* http.c */
int parse_url(const char *url, char *host, int *port, char *path, int *use_https);
int send_http_request(const char *host, int port, const char *path, const char *data, int use_https);

/* otel.c */
void json_escape_string(const char *input, char *output, int max_len);
void generate_metric_name_from_oid(const oid_t *oid, char *name, int max_len);
void snmp_value_to_string(const snmp_var_t *var, char *value_str, int max_len);
int build_otel_json(const snmp_var_t *vars, int var_count, char *json_buffer, int buffer_size);
int export_to_otel(const config_t *config, const snmp_var_t *vars, int var_count);

/* utils.c */
void log_message(int is_verbose, const char *format, ...);
void log_error(const char *format, ...);
unsigned long get_timestamp_ms(void);
int resolve_hostname(const char *hostname, char *ip_str);

#endif /* SNMP2OTEL_H */