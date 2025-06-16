/**
 * xbohach00
 * udp_scanner.c
 * Implementation of UDP scanning
 */

#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include "scanner.h"

/**
 * Scan a single UDP port
 */
port_status_t scan_udp_port(const char *ip_addr, const char *interface, uint16_t port, int timeout) {
	int sockfd;
	struct sockaddr_in dest_addr;
	char buffer[64] = "IPK-scanner probe packet";
	char recv_buffer[64];
	struct timeval tv;
	int result;
	
	// Create UDP socket
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("socket");
		return PORT_OPEN;
	}
	
	// Bind to specific interface if provided
	if (interface && *interface) {
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, interface, IF_NAMESIZE - 1);
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
			perror("setsockopt: SO_BINDTODEVICE");
			close(sockfd);
			return PORT_OPEN;
		}
	}
	
	// Set receive timeout
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt: SO_RCVTIMEO");
		close(sockfd);
		return PORT_OPEN;
	}
	
	// Set up destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	
	// Convert IP address
	int retry_count = 0;
	int pton_result = 0;
	while (retry_count < 3) {
		pton_result = inet_pton(AF_INET, ip_addr, &dest_addr.sin_addr);
		if (pton_result > 0) {
			break;
		}
		if (pton_result == 0) {
			// Not a valid IP address format
			close(sockfd);
			return PORT_OPEN;
		}
		// Error occurred, check if it's a temporary resource issue
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Wait a bit and retry
			usleep(10000);
			retry_count++;
			continue;
		}
		// Other error, not retryable
		perror("inet_pton");
		close(sockfd);
		return PORT_OPEN;
	}
	
	if (pton_result <= 0) {
		fprintf(stderr, "Failed to convert IP address after %d retries\n", retry_count);
		close(sockfd);
		return PORT_OPEN;
	}
	
	// Create a separate socket for checking ICMP port unreachable messages
	int sock_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_icmp < 0) {
		sock_icmp = -1;
	} else {
		if (setsockopt(sock_icmp, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
			perror("setsockopt: SO_RCVTIMEO on ICMP socket");
			close(sock_icmp);
			sock_icmp = -1;
		}
	}
	
	// Send the UDP packet
	if (sendto(sockfd, buffer, strlen(buffer), 0, 
			(struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		perror("sendto");
		if (sock_icmp >= 0) close(sock_icmp);
		close(sockfd);
		return PORT_OPEN;
	}
	
	result = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
	
	if (result >= 0) {
		// We got data back! Port is open.
		if (sock_icmp >= 0) close(sock_icmp);
		close(sockfd);
		return PORT_OPEN;
	} 
	
	if (errno == ECONNREFUSED) {
		// ECONNREFUSED means ICMP Port Unreachable was received
		if (sock_icmp >= 0) close(sock_icmp);
		close(sockfd);
		return PORT_CLOSED;
	}
	
	// Check for ICMP Port Unreachable messages
	if (sock_icmp >= 0) {
		char icmp_buffer[1500];
		
		// Try to receive ICMP message
		result = recv(sock_icmp, icmp_buffer, sizeof(icmp_buffer), 0);
		
		if (result > 0) {
			if (result >= 28 && icmp_buffer[20] == 3 && icmp_buffer[21] == 3) {
				close(sock_icmp);
				close(sockfd);
				return PORT_CLOSED;
			}
		}
		
		close(sock_icmp);
	}
	
	// If we get here, no response was received or timeout occurred
	close(sockfd);
	return PORT_OPEN;
}