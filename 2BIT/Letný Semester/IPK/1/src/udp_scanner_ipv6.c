/**
 * xbohach00
 * udp_scanner_ipv6.c
 * Implementation of UDP scanning for IPv6
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
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include "scanner.h"

/**
 * Scan a single UDP port using IPv6
 */
port_status_t scan_udp_port_ipv6(const char *ip_addr, const char *interface, uint16_t port, int timeout) {
	int sockfd, sockfd_icmp;
	struct sockaddr_in6 dest_addr;
	char buffer[64] = "IPK-scanner probe packet";
	char recv_buffer[1500];
	struct timeval tv;
	int result;
	int retries = 2;

	// Create UDP socket for IPv6
	sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("IPv6 UDP socket");
		return PORT_FILTERED;
	}

	// Bind to specific interface if provided
	if (interface && *interface) {
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, interface, IF_NAMESIZE - 1);
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
			perror("setsockopt: SO_BINDTODEVICE");
			close(sockfd);
			return PORT_FILTERED;
		}
	}

	// Set receive timeout
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt: SO_RCVTIMEO");
		close(sockfd);
		return PORT_FILTERED;
	}

	// Set up destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin6_family = AF_INET6;
	dest_addr.sin6_port = htons(port);

	// Convert IPv6 address
	int pton_result = inet_pton(AF_INET6, ip_addr, &dest_addr.sin6_addr);
	if (pton_result <= 0) {
		if (pton_result == 0)
			fprintf(stderr, "Not a valid IPv6 address: %s\n", ip_addr);
		else
			perror("inet_pton");
		close(sockfd);
		return PORT_FILTERED;
	}

	// Create a separate socket for ICMPv6 messages (port unreachable)
	sockfd_icmp = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sockfd_icmp < 0) {
		perror("IPv6 ICMP socket");
		sockfd_icmp = -1;
	} else {
		struct timeval icmp_tv;
		icmp_tv.tv_sec = (timeout * 1.5) / 1000;
		icmp_tv.tv_usec = ((timeout * 1.5) - (icmp_tv.tv_sec * 1000)) * 1000;
		
		if (setsockopt(sockfd_icmp, SOL_SOCKET, SO_RCVTIMEO, &icmp_tv, sizeof(icmp_tv)) < 0) {
			perror("setsockopt: SO_RCVTIMEO on ICMPv6 socket");
			close(sockfd_icmp);
			sockfd_icmp = -1;
		}
	}

	// Try with multiple retries
	for (int i = 0; i < retries; i++) {
		if (sendto(sockfd, buffer, strlen(buffer), 0, 
					(struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
			perror("sendto IPv6");
			if (sockfd_icmp >= 0) close(sockfd_icmp);
			close(sockfd);
			return PORT_FILTERED;
		}
		
		// First check: Try to receive a response on the UDP socket
		result = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
		
		if (result >= 0) {
			// We got data back! Port is open.
			if (sockfd_icmp >= 0) close(sockfd_icmp);
			close(sockfd);
			return PORT_OPEN;
		}
		
		if (errno == ECONNREFUSED) {
			if (sockfd_icmp >= 0) close(sockfd_icmp);
			close(sockfd);
			return PORT_CLOSED;
		}
		
		// If we have a raw ICMPv6 socket, check for ICMPv6 Destination Unreachable messages
		if (sockfd_icmp >= 0) {
			result = recv(sockfd_icmp, recv_buffer, sizeof(recv_buffer), 0);
			
			if (result > 0 && (size_t)result >= sizeof(struct icmp6_hdr)) {
				struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)recv_buffer;
				
				if (icmp6->icmp6_type == ICMP6_DST_UNREACH && icmp6->icmp6_code == ICMP6_DST_UNREACH_NOPORT) {            
					close(sockfd_icmp);
					close(sockfd);
					return PORT_CLOSED;
				}
			}
		}
		
		if (i < retries - 1) {
			usleep(100000);
		}
	}

	// Clean up
	if (sockfd_icmp >= 0) close(sockfd_icmp);
	close(sockfd);

	return PORT_OPEN;
}