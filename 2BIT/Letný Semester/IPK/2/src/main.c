/**
 * xbohach00
 * main.c - Main program entry
 */
#include "common.h"
#include "input.h"
#include "tcp_client.h"
#include "udp_client.h"
#include <signal.h>

// Global configuration
ClientConfig config;

// Termination flag for signal handlers
volatile sig_atomic_t termination_requested = 0;

void signal_handler(int sig);
void print_usage(const char *program_name);
bool parse_arguments(int argc, char *argv[]);

// Signal handler for graceful termination
void signal_handler(int sig) {
	if (sig == SIGINT || sig == SIGTERM) {
		printf_debug("Interrupt received, terminating gracefully...");
		termination_requested = 1;
		
		// Set a short alarm to exit if cleanup takes too long
		alarm(3);
		
		// Prevent immediate exit to allow for cleanup
		sleep(1);
	}
}

// Print program usage information
void print_usage(const char *program_name) {
	printf("Usage: %s [-t protocol] [-s server] [-p port] [-d timeout] [-r retries] [-h]\n\n", program_name);
	printf("  -t protocol   Transport protocol (tcp or udp) [REQUIRED]\n");
	printf("  -s server     Server IP address or hostname [REQUIRED]\n");
	printf("  -p port       Server port (default: %d)\n", DEFAULT_PORT);
	printf("  -d timeout    UDP confirmation timeout in milliseconds (default: %d)\n", DEFAULT_TIMEOUT_MS);
	printf("  -r retries    Maximum number of UDP retransmissions (default: %d)\n", DEFAULT_RETRIES);
	printf("  -h            Display this help message\n");
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[]) {
	int opt;
	bool has_protocol = false;
	bool has_server = false;
	
	// Set default values
	config.server_port = DEFAULT_PORT;
	config.timeout_ms = DEFAULT_TIMEOUT_MS;
	config.max_retries = DEFAULT_RETRIES;
	config.socket_fd = -1;
	config.server_dyn_port = 0;
	
	// Parse arguments
	while ((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1) {
		switch (opt) {
			case 't':
				has_protocol = true;
				if (strcmp(optarg, "tcp") == 0) {
					config.protocol = PROTO_TCP;
				} else if (strcmp(optarg, "udp") == 0) {
					config.protocol = PROTO_UDP;
				} else {
					fprintf(stderr, "Invalid protocol: %s (must be 'tcp' or 'udp')\n", optarg);
					return false;
				}
				break;
				
			case 's':
				has_server = true;
				config.server_host = optarg;
				break;
				
			case 'p':
				config.server_port = atoi(optarg);
				if (config.server_port <= 0 || config.server_port > 65535) {
					fprintf(stderr, "Invalid port number: %s\n", optarg);
					return false;
				}
				break;
				
			case 'd':
				config.timeout_ms = atoi(optarg);
				if (config.timeout_ms <= 0) {
					fprintf(stderr, "Invalid timeout value: %s\n", optarg);
					return false;
				}
				break;
				
			case 'r':
				config.max_retries = atoi(optarg);
				if (config.max_retries < 0) {
					fprintf(stderr, "Invalid retries value: %s\n", optarg);
					return false;
				}
				break;
				
			case 'h':
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);
				
			default:
				fprintf(stderr, "Unknown option: %c\n", opt);
				return false;
		}
	}
	
	// Check required arguments
	if (!has_protocol) {
		fprintf(stderr, "Missing required argument: -t (protocol)\n");
		return false;
	}
	
	if (!has_server) {
		fprintf(stderr, "Missing required argument: -s (server)\n");
		return false;
	}
	
	return true;
}

int main(int argc, char *argv[]) {
	// Setup signal handlers using signal() function which is more portable
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGALRM, signal_handler);
	
	// Parse command line arguments
	if (!parse_arguments(argc, argv)) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	printf_debug("Starting IPK25-CHAT client");
	
	// Initialize input handler
	init_input_handler();
	
	// Initialize client based on selected protocol
	bool init_success = false;
	if (config.protocol == PROTO_TCP) {
		printf_debug("Initializing TCP client");
		init_success = tcp_client_init(config.server_host, config.server_port);
	} else {
		printf_debug("Initializing UDP client");
		init_success = udp_client_init(config.server_host, config.server_port, 
									config.timeout_ms, config.max_retries);
	}
	
	if (!init_success) {
		printf_debug("Client initialization failed");
		return EXIT_FAILURE;
	}
	
	// Run client main loop
	if (config.protocol == PROTO_TCP) {
		printf_debug("Running TCP client");
		tcp_client_run();
		tcp_client_cleanup();
	} else {
		printf_debug("Running UDP client");
		udp_client_run();
		udp_client_cleanup();
	}
	
	printf_debug("Client terminated successfully");
	return EXIT_SUCCESS;
}