/**
 * xbohach00
 * main.c
 * Main entry point for the L4 scanner application
 */

#define _POSIX_C_SOURCE 200809L

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "scanner.h"
#include "util.h"


// Global flag for handling Ctrl+C
volatile sig_atomic_t stop_flag = 0;

/**
 * Signal handler for SIGINT - (Ctrl+C) usefull when debugging with long inputs
 */
void signal_handler(int signum) {
	if (signum == SIGINT) {
		stop_flag = 1;
	}
}

/**
 * Print the usage instructions
 */
void print_usage() {
	printf("Usage: ipk-l4-scan [-i interface | --interface interface] [--pu port-ranges | --pt port-ranges | -u port-ranges | -t port-ranges] {-w timeout} [hostname | ip-address]\n");
	printf("       ipk-l4-scan --help\n");
	printf("       ipk-l4-scan --interface\n");
	printf("       ipk-l4-scan\n\n");
	printf("Options:\n");
	printf("  -h, --help                 Display this help message\n");
	printf("  -i, --interface INTERFACE  Network interface to use for scanning\n");
	printf("  -t, --pt PORT-RANGES       TCP ports to scan (e.g., 22 or 1-65535 or 22,23,24)\n");
	printf("  -u, --pu PORT-RANGES       UDP ports to scan (e.g., 53,67 or 1-1000)\n");
	printf("  -w, --wait TIMEOUT         Timeout in milliseconds (default: 5000)\n");
}

int main(int argc, char *argv[]) {
	// Ctrl+C handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, NULL);
	
	// Default parameter initialization
	char *interface = NULL;
	char *tcp_ports = NULL;
	char *udp_ports = NULL;
	char *target = NULL;
	int timeout = 5000;
	
	// Parse command line arguments
	int c;
	int show_help = 0; // Needed since help should disregard all other options
	
	int show_interfaces = 0;
	if (argc == 2) {
		if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interface") == 0) {
			show_interfaces = 1;
		}
	}
	
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"interface", required_argument, 0, 'i'},
		{"pt", required_argument, 0, 'P'},
		{"pu", required_argument, 0, 'U'},
		{"wait", required_argument, 0, 'w'},
		{0, 0, 0, 0}
	};
	
	int option_index = 0;
	
	while ((c = getopt_long(argc, argv, "hi:t:u:w:", long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				show_help = 1;
				break;
			case 'i':
				interface = optarg;
				break;
			case 't':
			case 'P': // --pt
				tcp_ports = optarg;
				break;
			case 'u':
			case 'U': // --pu
				udp_ports = optarg;
				break;
			case 'w':
				timeout = atoi(optarg);
				if (timeout <= 0) {
					fprintf(stderr, "Error: Invalid timeout value\n");
					return EXIT_FAILURE;
				}
				break;
			case '?':
				if (optopt == 'i' || strcmp(argv[optind-1], "--interface") == 0) {
					show_interfaces = 1;
					break;
				}
				fprintf(stderr, "Try 'ipk-l4-scan --help' for more information.\n");
				return EXIT_FAILURE;
			default:
				fprintf(stderr, "Invalid option\n");
				print_usage();
				return EXIT_FAILURE;
		}
	}
	
	// Handle special cases
	if (show_help) {
		print_usage();
		return EXIT_SUCCESS;
	}
	
	if (show_interfaces || argc == 1) {
		print_interfaces();
		return EXIT_SUCCESS;
	}
	
	// Set target
	if (optind < argc) {
		target = argv[optind];
	}
	
	// Do we have all required parameters?
	if (!interface || (!tcp_ports && !udp_ports) || !target) {
		fprintf(stderr, "Error: Missing required parameters\n");
		print_usage();
		return EXIT_FAILURE;
	}
	
	// Start scanning
	scan_target(interface, tcp_ports, udp_ports, target, timeout, &stop_flag);
	
	return EXIT_SUCCESS;
}