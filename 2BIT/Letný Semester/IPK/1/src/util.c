/**
 * xbohach00
 * utils.c
 * Implementation of utility functions
 */

 #include <ifaddrs.h>
 #include <netdb.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include "util.h"

/**
 * Print all available network interfaces
 */
void print_interfaces() {
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return;
	}
	
	printf("Available interfaces:\n");
	
	// Walk through linked list of interfaces
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		
		family = ifa->ifa_addr->sa_family;
		
		// For IPv4 or IPv6
		if (family == AF_INET || family == AF_INET6) {
			s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				continue;
			}
			
			printf("%-8s %s (%s)\n", ifa->ifa_name, (family == AF_INET) ? "IPv4" : "IPv6", host);
		}
	}
	
	freeifaddrs(ifaddr);
}

/**
 * Resolve hostname to a list of IP addresses
 */
int resolve_hostname(const char *hostname, char addresses[][INET6_ADDRSTRLEN], int max_addresses, int *is_ipv6) {
	struct addrinfo hints, *result, *rp;
	int count = 0;
	
	// Check if hostname is already an IPv6 address (contains ':')
	if (strchr(hostname, ':') != NULL) {
		strncpy(addresses[0], hostname, INET6_ADDRSTRLEN - 1);
		addresses[0][INET6_ADDRSTRLEN - 1] = '\0';
		is_ipv6[0] = 1;
		return 1;
	}
	
	// Check if hostname is already an IPv4 address (xxx.xxx.xxx.xxx format)
	struct in_addr ipv4_addr;
	if (inet_pton(AF_INET, hostname, &ipv4_addr) == 1) {
		strncpy(addresses[0], hostname, INET6_ADDRSTRLEN - 1);
		addresses[0][INET6_ADDRSTRLEN - 1] = '\0';
		is_ipv6[0] = 0;
		return 1;
	}
	
	// Initialize hints for hostname resolution
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP socket
	
	// Resolve the domain name
	int s = getaddrinfo(hostname, NULL, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return 0;
	}
	
	// Process each address
	for (rp = result; rp != NULL && count < max_addresses; rp = rp->ai_next) {
		void *addr;
		
		// Get the address based on the family
		if (rp->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
			addr = &(ipv4->sin_addr);
			is_ipv6[count] = 0;
		} else if (rp->ai_family == AF_INET6) { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
			addr = &(ipv6->sin6_addr);
			is_ipv6[count] = 1;
		} else {
			continue;
		}
		
		inet_ntop(rp->ai_family, addr, addresses[count], INET6_ADDRSTRLEN);
		
		if (strchr(addresses[count], ':') != NULL) {
			is_ipv6[count] = 1;
		}
		
		count++;
	}
	
	freeaddrinfo(result);
	return count;
}