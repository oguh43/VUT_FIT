/**
 * xbohach00
 * utils.h
 * Utility functions for the L4 scanner
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* For INET6_ADDRSTRLEN */

/**
 * Print all available network interfaces
 */
void print_interfaces();

/**
 * Resolve hostname to a list of IP addresses
 * 
 * @param hostname The hostname to resolve
 * @param addresses Array to store the resolved addresses
 * @param max_addresses Maximum number of addresses to store
 * @param is_ipv6 Array to store whether each address is IPv6 (1) or IPv4 (0)
 * @return Number of resolved addresses
 */
int resolve_hostname(const char *hostname, char addresses[][INET6_ADDRSTRLEN], int max_addresses, int *is_ipv6);

#endif // UTILS_H