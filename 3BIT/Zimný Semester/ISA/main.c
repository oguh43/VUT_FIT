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
*       Implementation of the SNMP to OpenTelemetry exporter main program      *
*                                                                              *
*******************************************************************************/

#include <time.h>
#include <getopt.h>
#include "snmp2otel.h"

volatile int running = 1;

void print_usage(const char *prog_name) {
	fprintf(stderr, "Usage: %s -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-v]\n", prog_name);
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "  -t target     IP address or DNS name of SNMP agent\n");
	fprintf(stderr, "  -C community  SNMP v2c community string (default: public)\n");
	fprintf(stderr, "  -o oids_file  File with list of OIDs to query\n");
	fprintf(stderr, "  -e endpoint   OTEL endpoint URL for export\n");
	fprintf(stderr, "  -i interval   Query period in seconds (default: 10)\n");
	fprintf(stderr, "  -r retries    Number of retries on timeout (default: 2)\n");
	fprintf(stderr, "  -T timeout    SNMP query timeout in ms (default: 1000)\n");
	fprintf(stderr, "  -p port       UDP port for SNMP agent (default: 161)\n");
	fprintf(stderr, "  -v            Verbose mode\n");
}

int parse_arguments(int argc, char *argv[], config_t *config) {
	int opt;
	int target_set = 0, oids_file_set = 0, endpoint_set = 0;
	
	/* Initialize config with defaults */
	memset(config, 0, sizeof(config_t));
	strcpy(config->community, DEFAULT_COMMUNITY);
	config->interval = DEFAULT_INTERVAL;
	config->retries = DEFAULT_RETRIES;
	config->timeout = DEFAULT_TIMEOUT;
	config->port = DEFAULT_PORT;
	config->verbose = 0;
	
	while ((opt = getopt(argc, argv, "t:C:o:e:i:r:T:p:vh")) != -1) {
		switch (opt) {
			case 't':
				strncpy(config->target, optarg, MAX_HOSTNAME_LEN - 1);
				config->target[MAX_HOSTNAME_LEN - 1] = '\0';
				target_set = 1;
				break;
			case 'C':
				strncpy(config->community, optarg, MAX_COMMUNITY_LEN - 1);
				config->community[MAX_COMMUNITY_LEN - 1] = '\0';
				break;
			case 'o':
				strncpy(config->oids_file, optarg, MAX_LINE_LEN - 1);
				config->oids_file[MAX_LINE_LEN - 1] = '\0';
				oids_file_set = 1;
				break;
			case 'e':
				strncpy(config->endpoint, optarg, MAX_URL_LEN - 1);
				config->endpoint[MAX_URL_LEN - 1] = '\0';
				endpoint_set = 1;
				break;
			case 'i':
				config->interval = atoi(optarg);
				if (config->interval <= 0) {
					log_error("Invalid interval: %s\n", optarg);
					return -1;
				}
				break;
			case 'r':
				config->retries = atoi(optarg);
				if (config->retries < 0) {
					log_error("Invalid retries: %s\n", optarg);
					return -1;
				}
				break;
			case 'T':
				config->timeout = atoi(optarg);
				if (config->timeout <= 0) {
					log_error("Invalid timeout: %s\n", optarg);
					return -1;
				}
				break;
			case 'p':
				config->port = atoi(optarg);
				if (config->port <= 0 || config->port > 65535) {
					log_error("Invalid port: %s\n", optarg);
					return -1;
				}
				break;
			case 'v':
				config->verbose = 1;
				verbose_mode = 1;
				break;
			case 'h':
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);
			default:
				print_usage(argv[0]);
				return -1;
		}
	}
	
	/* Check required parameters */
	if (!target_set) {
		log_error("Target (-t) is required\n");
		return -1;
	}
	
	if (!oids_file_set) {
		log_error("OIDs file (-o) is required\n");
		return -1;
	}
	
	if (!endpoint_set) {
		log_error("OTEL endpoint (-e) is required\n");
		return -1;
	}
	
	return 0;
}

void signal_handler(int sig) {
	if (sig == SIGINT || sig == SIGTERM) {
		log_message(1, "Received signal %d, shutting down...\n", sig);
		running = 0;
	}
}

int main(int argc, char *argv[]) {
	config_t config;
	oid_list_t oids;
	snmp_var_t results[MAX_OID_COUNT];
	int result_count;
	time_t last_query = 0;
	int query_failed_count = 0;
	
	/* Parse command line arguments */
	if (parse_arguments(argc, argv, &config) != 0) {
		return EXIT_FAILURE;
	}
	
	/* Set up signal handlers */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	
	/* Load OIDs from file */
	log_message(config.verbose, "Loading OIDs from file: %s\n", config.oids_file);
	if (load_oids_from_file(config.oids_file, &oids) != 0) {
		log_error("Failed to load OIDs from file: %s\n", config.oids_file);
		return EXIT_FAILURE;
	}
	
	log_message(config.verbose, "Loaded %d OIDs\n", oids.count);
	if (config.verbose) {
		for (int i = 0; i < oids.count; i++) {
			log_message(1, "OID %d: %s\n", i + 1, oids.oids[i].str);
		}
	}
	
	log_message(config.verbose, "Starting SNMP to OTEL exporter\n");
	log_message(config.verbose, "Target: %s:%d\n", config.target, config.port);
	log_message(config.verbose, "Community: %s\n", config.community);
	log_message(config.verbose, "Endpoint: %s\n", config.endpoint);
	log_message(config.verbose, "Interval: %d seconds\n", config.interval);
	log_message(config.verbose, "Retries: %d\n", config.retries);
	log_message(config.verbose, "Timeout: %d ms\n", config.timeout);
	
	/* Main loop */
	while (running) {
		time_t current_time;
		current_time = time(NULL);
		
		/* Check if it's time for the next query */
		if (current_time - last_query >= config.interval) {
			log_message(config.verbose, "Performing SNMP query...\n");
			
			/* Send SNMP request */
			result_count = 0;
			if (send_snmp_request(&config, &oids, results, &result_count) == 0) {
				if (result_count > 0) {
					log_message(config.verbose, "Received %d SNMP responses\n", result_count);
					
					/* Export to OpenTelemetry */
					if (export_to_otel(&config, results, result_count) == 0) {
						log_message(config.verbose, "Successfully exported metrics to OTEL\n");
						query_failed_count = 0;
					} else {
						log_error("Failed to export metrics to OTEL\n");
						query_failed_count++;
					}
				} else {
					log_error("No valid SNMP responses received\n");
					query_failed_count++;
				}
			} else {
				log_error("SNMP query failed\n");
				query_failed_count++;
			}
			
			last_query = current_time;
			
			if (query_failed_count > 0) {
				log_message(config.verbose, "Query failures: %d\n", query_failed_count);
			}
		}
		
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 100000000; /* 100ms */
		nanosleep(&sleep_time, NULL);
	}
	
	log_message(config.verbose, "SNMP to OTEL exporter stopped\n");
	return EXIT_SUCCESS;
}