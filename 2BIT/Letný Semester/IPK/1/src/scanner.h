/**
 * xbohach00
 * scanner.h
 * Interface for the L4 port scanner operations
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>
#include <signal.h>

// Port status definitions
typedef enum {
	PORT_OPEN,
	PORT_CLOSED,
	PORT_FILTERED
} port_status_t;

// Main scan function - creates threads for each IP address and protocol and handles resolving hostnames
void scan_target(const char *interface, const char *tcp_ports, const char *udp_ports, const char *target, int timeout, volatile sig_atomic_t *stop_flag);

// Function to print the result of a scan
void print_scan_result(const char *ip_addr, uint16_t port, const char *protocol, port_status_t status);

// Functions for TCP and UDP scanning (IPv4)
port_status_t scan_tcp_port(const char *ip_addr, const char *interface, uint16_t port, int timeout);
port_status_t scan_udp_port(const char *ip_addr, const char *interface, uint16_t port, int timeout);

// Functions for TCP and UDP scanning (IPv6)
port_status_t scan_tcp_port_ipv6(const char *ip_addr, const char *interface, uint16_t port, int timeout);
port_status_t scan_udp_port_ipv6(const char *ip_addr, const char *interface, uint16_t port, int timeout);

#endif // SCANNER_H
