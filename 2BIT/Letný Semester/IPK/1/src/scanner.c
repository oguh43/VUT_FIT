/**
 * xbohach00
 * scanner.c
 * Implementation of the scanner functionality with multithreading
 */

#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include "scanner.h"
#include "util.h"

#define MAX_ADDRESSES 10

// Structure to represent a scan task for a single protocol (TCP or UDP)
typedef struct {
	char ip_addr[INET6_ADDRSTRLEN];
	char interface[IF_NAMESIZE];
	char *port_ranges;
	char protocol[8];
	int timeout;
	int is_ipv6;
	volatile sig_atomic_t *stop_flag;
} scan_protocol_task_t;

/**
 * Print scan result in specified format
 */
void print_scan_result(const char *ip_addr, uint16_t port, const char *protocol, port_status_t status) {
	const char *status_str;
	
	switch (status) {
		case PORT_OPEN:
			status_str = "open";
			break;
		case PORT_CLOSED:
			status_str = "closed";
			break;
		case PORT_FILTERED:
			status_str = "filtered";
			break;
		default:
			status_str = "unknown";
	}
	
	printf("%s %d %s %s\n", ip_addr, port, protocol, status_str);
}

/**
 * Parses port ranges and call scanning function for each port
 */
static void scan_ports(const char *ip_addr, const char *interface, const char *port_ranges, const char *protocol, int timeout, volatile sig_atomic_t *stop_flag, int is_ipv6) {
	char *port_str = strdup(port_ranges);
	char *token, *saveptr;

	token = strtok_r(port_str, ",", &saveptr);

	while (token && !*stop_flag) {
		// Handle ranges
		char *hyphen = strchr(token, '-');
		if (hyphen) {
			*hyphen = '\0';
			int start_port = atoi(token);
			int end_port = atoi(hyphen + 1);

			if (start_port < 1 || start_port > 65535 || end_port < 1 || end_port > 65535 || start_port > end_port) {
				fprintf(stderr, "Error: Invalid port range %s-%s\n", token, hyphen + 1);
				token = strtok_r(NULL, ",", &saveptr);
				continue;
			}

			for (int port = start_port; port <= end_port && !*stop_flag; port++) {
				port_status_t status = PORT_FILTERED;

				if (strcmp(protocol, "tcp") == 0) {
					if (is_ipv6) {
						status = scan_tcp_port_ipv6(ip_addr, interface, (uint16_t)port, timeout);
					} else {
						status = scan_tcp_port(ip_addr, interface, (uint16_t)port, timeout);
					}
				} else {
					if (is_ipv6) {
						status = scan_udp_port_ipv6(ip_addr, interface, (uint16_t)port, timeout);
					} else {
						status = scan_udp_port(ip_addr, interface, (uint16_t)port, timeout);
					}
				}

				print_scan_result(ip_addr, port, protocol, status);

				// Small delay to avoid overwhelming the system resources - network libraries threw erorrs without this
				usleep(50000);
			}
		} else {

			// Single port
			int port = atoi(token);

			if (port < 1 || port > 65535) {
				fprintf(stderr, "Error: Invalid port %s\n", token);
				token = strtok_r(NULL, ",", &saveptr);
				continue;
			}

			port_status_t status = PORT_FILTERED;

			if (strcmp(protocol, "tcp") == 0) {
				if (is_ipv6) {
					status = scan_tcp_port_ipv6(ip_addr, interface, (uint16_t)port, timeout);
				} else {
					status = scan_tcp_port(ip_addr, interface, (uint16_t)port, timeout);
				}
			} else {
				if (is_ipv6) {
					status = scan_udp_port_ipv6(ip_addr, interface, (uint16_t)port, timeout);
				} else {
					status = scan_udp_port(ip_addr, interface, (uint16_t)port, timeout);
				}
			}

			print_scan_result(ip_addr, port, protocol, status);

			usleep(50000);
		}

		token = strtok_r(NULL, ",", &saveptr);
	}

	free(port_str);
}

/**
 * Thread function for scanning one protocol (TCP or UDP)
 */
void *scan_protocol_thread(void *arg) {
	scan_protocol_task_t *task = (scan_protocol_task_t *)arg;
	
	scan_ports(task->ip_addr, task->interface, task->port_ranges, task->protocol, task->timeout, task->stop_flag, task->is_ipv6);
	
	free(task->port_ranges);
	free(task);
	return NULL;
}

/**
 * Main scan function - creates threads for each IP address and protocol and handles resolving hostnames
 */
void scan_target(const char *interface, const char *tcp_ports, const char *udp_ports, const char *target, int timeout, volatile sig_atomic_t *stop_flag) {
	
	// Resolve hostname to IP addresses
	char addresses[MAX_ADDRESSES][INET6_ADDRSTRLEN];
	int is_ipv6[MAX_ADDRESSES];
	
	int num_addresses = resolve_hostname(target, addresses, MAX_ADDRESSES, is_ipv6);
	
	if (num_addresses == 0) {
		fprintf(stderr, "Error: Could not resolve hostname '%s'\n", target);
		return;
	}
	
	// Create threads for each IP address and protocol
	pthread_t threads[MAX_ADDRESSES * 2];
	int thread_count = 0;
	
	// Scan each resolved IP address
	for (int i = 0; i < num_addresses && !*stop_flag; i++) {
		const char *ip_addr = addresses[i];
		
		// Create a thread for TCP ports if specified
		if (tcp_ports && !*stop_flag) {
			scan_protocol_task_t *task = (scan_protocol_task_t *)malloc(sizeof(scan_protocol_task_t));
			if (!task) {
				perror("malloc");
				continue; // May god be with us and grant aditional memory on the next iteration
			}
			
			strncpy(task->ip_addr, ip_addr, INET6_ADDRSTRLEN - 1);
			task->ip_addr[INET6_ADDRSTRLEN - 1] = '\0';
			strncpy(task->interface, interface, IF_NAMESIZE - 1);
			task->interface[IF_NAMESIZE - 1] = '\0';
			task->port_ranges = strdup(tcp_ports);
			strncpy(task->protocol, "tcp", 7);
			task->protocol[7] = '\0';
			task->timeout = timeout;
			task->is_ipv6 = is_ipv6[i];
			task->stop_flag = stop_flag;
			
			if (pthread_create(&threads[thread_count], NULL, scan_protocol_thread, task) != 0) {
				perror("pthread_create");
				free(task->port_ranges);
				free(task);
			} else {
				thread_count++;
			}
		}
		
		// Create thread for UDP ports if specified
		if (udp_ports && !*stop_flag) {
			scan_protocol_task_t *task = (scan_protocol_task_t *)malloc(sizeof(scan_protocol_task_t));
			if (!task) {
				perror("malloc");
				continue; // May god be with us and grant aditional memory on the next iteration
			}
			
			strncpy(task->ip_addr, ip_addr, INET6_ADDRSTRLEN - 1);
			task->ip_addr[INET6_ADDRSTRLEN - 1] = '\0';
			strncpy(task->interface, interface, IF_NAMESIZE - 1);
			task->interface[IF_NAMESIZE - 1] = '\0';
			task->port_ranges = strdup(udp_ports);
			strncpy(task->protocol, "udp", 7);
			task->protocol[7] = '\0';
			task->timeout = timeout;
			task->is_ipv6 = is_ipv6[i];
			task->stop_flag = stop_flag;
			
			if (pthread_create(&threads[thread_count], NULL, scan_protocol_thread, task) != 0) {
				perror("pthread_create");
				free(task->port_ranges);
				free(task);
			} else {
				thread_count++;
			}
		}
	}
	
	// Wait for all threads to complete
	for (int i = 0; i < thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
}