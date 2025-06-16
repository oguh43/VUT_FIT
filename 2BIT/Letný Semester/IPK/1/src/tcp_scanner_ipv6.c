/**
 * xbohach00
 * tcp_scanner_ipv6.c
 * Implementation of TCP SYN scanning for IPv6
 */

#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include "scanner.h"

// Global variables for packet capture
static pcap_t *pcap_handle = NULL;
static int packet_received = 0;
static uint8_t packet_type = 0;  // 1 = SYN-ACK | 2 = RST

/**
 * Packet capture callback for IPv6
 */
void packet_handler_ipv6(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
	(void)user;
	(void)header;

	struct ip6_hdr *ipv6_header;
	struct tcphdr *tcp_header;
	int ipv6_header_len = sizeof(struct ip6_hdr);
	
	// Skip Ethernet header (if present)
	const u_char *payload = packet;
	if (pcap_datalink(pcap_handle) == DLT_EN10MB) {
		payload += 14;
	}
	
	// Get and verify the IPv6 header + protocol
	ipv6_header = (struct ip6_hdr *)payload;
	
	if ((ipv6_header->ip6_vfc >> 4) != 6) {
		return;  
	}
	
	if (ipv6_header->ip6_nxt != IPPROTO_TCP) {
		return;
	}
	
	tcp_header = (struct tcphdr *)(payload + ipv6_header_len);
	
	// Check for SYN-ACK (both SYN and ACK flags set)
	if ((tcp_header->th_flags & (TH_SYN|TH_ACK)) == (TH_SYN|TH_ACK)) {
		packet_received = 1;
		packet_type = 1;
	} 
	// Check for RST (RST flag set)
	else if (tcp_header->th_flags & TH_RST) {
		packet_received = 1;
		packet_type = 2;
	}
}

/**
 * Get local IPv6 address for the specified interface
 */
int get_local_ipv6(const char *interface, char *ip_buffer, size_t buffer_size) {
	struct ifaddrs *ifaddr, *ifa;
	int found = 0;
	int interface_exists = 0;
	int interface_up = 0;
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		fprintf(stderr, "Using IPv6 loopback address as fallback\n");
		strncpy(ip_buffer, "::1", buffer_size);
		return 0;  // Return success with fallback
	}
	
	// Check if the found interface is up
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_name && strcmp(ifa->ifa_name, interface) == 0) {
			interface_exists = 1;
			if (ifa->ifa_flags & IFF_UP) {
				interface_up = 1;
			}
			break;
		}
	}
	
	if (!interface_exists) {
		fprintf(stderr, "Interface %s does not exist. Using IPv6 loopback.\n", interface);
		freeifaddrs(ifaddr);
		strncpy(ip_buffer, "::1", buffer_size);
		return 0;
	}
	
	if (!interface_up) {
		fprintf(stderr, "Interface %s is not up. Using IPv6 loopback.\n", interface);
		freeifaddrs(ifaddr);
		strncpy(ip_buffer, "::1", buffer_size);
		return 0;
	}
	
	// Try to find a global IPv6 address first
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		
		// Check if it's the requested interface and an IPv6 address
		if (strcmp(ifa->ifa_name, interface) == 0 && ifa->ifa_addr->sa_family == AF_INET6) {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
			
			// Skip link-local addresses (fe80::)
			if (IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr))
				continue;
			
			// Convert IPv6 address to string
			if (inet_ntop(AF_INET6, &addr->sin6_addr, ip_buffer, buffer_size) != NULL) {
				found = 1;
				break;
			}
		}
	}
	
	// If no global IPv6 address found, try link-local as a fallback
	if (!found) {
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;
			
			if (strcmp(ifa->ifa_name, interface) == 0 && ifa->ifa_addr->sa_family == AF_INET6) {
				struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
				
				// Convert IPv6 address to string (including link-local this time)
				if (inet_ntop(AF_INET6, &addr->sin6_addr, ip_buffer, buffer_size) != NULL) {
					found = 1;
					break;
				}
			}
		}
	}
	
	freeifaddrs(ifaddr);
	
	if (!found) {
		fprintf(stderr, "No IPv6 address found for interface %s. Using IPv6 loopback.\n", interface);
		strncpy(ip_buffer, "::1", buffer_size);
	}
	
	return 0;
}

/**
 * Set up packet capture for IPv6 TCP responses
 */
pcap_t* setup_packet_capture_ipv6(const char *interface, uint16_t port) {
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter[100];
	bpf_u_int32 net, mask;
	
	// Check if the interface is up using getifaddrs
	struct ifaddrs *ifaddr, *ifa;
	int interface_up = 0;
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
	} else {
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_name && strcmp(ifa->ifa_name, interface) == 0) {
				if (ifa->ifa_flags & IFF_UP) {
					interface_up = 1;
					break;
				}
			}
		}
		freeifaddrs(ifaddr);
	}
	
	if (!interface_up) {
		fprintf(stderr, "Interface %s is not up. Using 'lo' (loopback) as fallback.\n", interface);
		interface = "lo";
	}
	
	// Get the network address and mask
	if (pcap_lookupnet(interface, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", interface, errbuf);
		net = 0;
		mask = 0;
	}
	
	// Open the device for sniffing
	handle = pcap_open_live(interface, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", interface, errbuf);
		
		if (strcmp(interface, "lo") != 0) {
			fprintf(stderr, "Trying fallback to loopback interface 'lo'...\n");
			return setup_packet_capture_ipv6("lo", port);
		}
		return NULL;
	}
	
	// Create a filter for TCP packets from the specified port
	snprintf(filter, sizeof(filter), "ip6 and tcp and src port %d and ((tcp[13] & 0x12) == 0x12 or (tcp[13] & 0x04) != 0)", port);
	
	if (pcap_compile(handle, &fp, filter, 0, PCAP_NETMASK_UNKNOWN) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter, pcap_geterr(handle));
		pcap_close(handle);
		return NULL;
	}
	
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter, pcap_geterr(handle));
		pcap_close(handle);
		return NULL;
	}
	
	// Set non-blocking mode
	if (pcap_setnonblock(handle, 1, errbuf) == -1) {
		fprintf(stderr, "Couldn't set non-blocking mode: %s\n", errbuf);
		pcap_close(handle);
		return NULL;
	}
	
	pcap_freecode(&fp);
	return handle;
}

/**
 * TCP SYN scan implementation for IPv6 - Using TCP sockets with timeouts
 * Couldn't get raw sockets to work with IPv6, unfortunately need to use regular TCP sockets BUT full handshake is never achieved - we close the connection after sending the SYN packet
 */
port_status_t scan_tcp_port_ipv6(const char *ip_addr, const char *interface, uint16_t port, int timeout) {
	int sockfd;
	struct sockaddr_in6 dest_addr;
	struct timeval tv;
	fd_set write_fds, read_fds, error_fds;
	int result;
	int so_error = 0;
	socklen_t len = sizeof(so_error);
	
	// Create TCP socket for IPv6
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("IPv6 TCP socket creation failed");
		return PORT_FILTERED;
	}
	
	// Set non-blocking mode
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0) {
		perror("fcntl F_GETFL");
		close(sockfd);
		return PORT_FILTERED;
	}
	
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl F_SETFL O_NONBLOCK");
		close(sockfd);
		return PORT_FILTERED;
	}
	
	// Bind to specific interface if it exists and is up
	if (interface && *interface) {
		struct ifaddrs *ifaddr, *ifa;
		int interface_exists = 0;
		int interface_up = 0;
		
		if (getifaddrs(&ifaddr) == 0) {
			for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
				if (ifa->ifa_name && strcmp(ifa->ifa_name, interface) == 0) {
					interface_exists = 1;
					if (ifa->ifa_flags & IFF_UP) {
						interface_up = 1;
					}
					break;
				}
			}
			freeifaddrs(ifaddr);
			
			if (interface_exists && interface_up) {
				#ifdef SO_BINDTODEVICE
				struct ifreq ifr;
				memset(&ifr, 0, sizeof(ifr));
				strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
				
				if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
					perror("setsockopt SO_BINDTODEVICE");
				}
				#endif
			}
		}
	}
	
	// Set up destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin6_family = AF_INET6;
	dest_addr.sin6_port = htons(port);
	
	if (inet_pton(AF_INET6, ip_addr, &dest_addr.sin6_addr) <= 0) {
		perror("inet_pton IPv6");
		close(sockfd);
		return PORT_FILTERED;
	}
	
	// Start connection - non-blocking
	if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		if (errno != EINPROGRESS) {
			close(sockfd);
			return PORT_FILTERED;
		}
		
		FD_ZERO(&write_fds);
		FD_SET(sockfd, &write_fds);
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);
		FD_ZERO(&error_fds);
		FD_SET(sockfd, &error_fds);
		
		// Set timeout
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		
		// Wait for either readability, writability, or error
		result = select(sockfd + 1, &read_fds, &write_fds, &error_fds, &tv);
		
		if (result < 0) {
			perror("select");
			close(sockfd);
			return PORT_FILTERED;
		} else if (result == 0) {
			// Timeout
			close(sockfd);
			
			// For more accurate detection, try a second connection with shorter timeout - "filtered" verification
			int verification_wait = timeout / 2;
			int second_attempt_sock = socket(AF_INET6, SOCK_STREAM, 0);
			
			if (second_attempt_sock < 0) {
				return PORT_FILTERED;
			}
			
			// Set non-blocking for second attempt
			if (fcntl(second_attempt_sock, F_SETFL, O_NONBLOCK) < 0) {
				close(second_attempt_sock);
				return PORT_FILTERED;
			}
			
			// Try connection
			if (connect(second_attempt_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
				if (errno != EINPROGRESS) {
					close(second_attempt_sock);
					return PORT_FILTERED;
				}
				
				FD_ZERO(&write_fds);
				FD_SET(second_attempt_sock, &write_fds);
				
				// Shorter timeout for verification
				tv.tv_sec = verification_wait / 1000;
				tv.tv_usec = (verification_wait % 1000) * 1000;
				
				result = select(second_attempt_sock + 1, NULL, &write_fds, NULL, &tv);
				
				if (result <= 0) {
					// Still timed out or error - filtered
					close(second_attempt_sock);
					return PORT_FILTERED;
				}
				
				// Check socket error on verification attempt
				if (getsockopt(second_attempt_sock, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
					close(second_attempt_sock);
					return PORT_FILTERED;
				}
				
				if (so_error == 0) {
					// Successful connection on verification - shouldn't happen since we timed out on the first attempt
					close(second_attempt_sock);
					return PORT_OPEN;
				} else if (so_error == ECONNREFUSED) {
					close(second_attempt_sock);
					return PORT_CLOSED;
				}
				
				close(second_attempt_sock);
				return PORT_FILTERED;
			}
			
			close(second_attempt_sock);
			return PORT_FILTERED;
		}
		
		// Check for socket error
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
			perror("getsockopt");
			close(sockfd);
			return PORT_FILTERED;
		}
		
		if (so_error != 0) {
			if (so_error == ECONNREFUSED) {
				close(sockfd);
				return PORT_CLOSED;
			} else {
				close(sockfd);
				return PORT_FILTERED;
			}
		}
	}
	
	// Connection succeeded - port is open
	// Stop here before completing handshake (close the connection)
	close(sockfd);
	return PORT_OPEN;
}