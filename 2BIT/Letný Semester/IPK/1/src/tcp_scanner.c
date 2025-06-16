/**
 * xbohach00
 * tcp_scanner.c
 * Implementation of TCP SYN scanning (non-blocking version)
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

// Pseudo header for TCP checksum calculation
struct pseudo_header {
	uint32_t source_address;
	uint32_t dest_address;
	uint8_t placeholder;
	uint8_t protocol;
	uint16_t tcp_length;
};

// Global variables for packet capture
static pcap_t *pcap_handle = NULL;
static int packet_received = 0;
static uint8_t packet_type = 0;  // 1 = SYN-ACK | 2 = RST
static struct timeval start_time;

/**
 * Calculate TCP checksum
 */
uint16_t calculate_tcp_checksum(const void *buff, int length, uint32_t src_addr, uint32_t dest_addr) {
	const uint16_t *buf = buff;
	uint32_t sum = 0;
	int i;
	
	// Pseudo header
	struct pseudo_header psh;
	psh.source_address = src_addr;
	psh.dest_address = dest_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(length);
	
	const uint16_t *pseudo = (const uint16_t *)&psh;
	for (i = 0; (size_t) i < sizeof(struct pseudo_header) / 2; i++) {
		sum += pseudo[i];
	}
	
	// Add the TCP header and data
	for (i = 0; i < length / 2; i++) {
		sum += buf[i];
	}
	
	// If length is odd, add the last byte
	if (length % 2 != 0) {
		sum += ((uint8_t *)buf)[length - 1];
	}
	
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	
	return ~sum;
}

/**
 * Packet capture callback
 */
void packet_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
	(void)user;
	(void)header;
	struct ip *ip_header;
	struct tcphdr *tcp_header;
	int ip_header_len;
	
	// Skip Ethernet header (if present)
	const u_char *payload = packet;
	if (pcap_datalink(pcap_handle) == DLT_EN10MB) {
		payload += 14;
	}
	
	// Get the IP header
	ip_header = (struct ip *)payload;
	ip_header_len = ip_header->ip_hl * 4;
	
	// Get the TCP header
	tcp_header = (struct tcphdr *)(payload + ip_header_len);
	
	if ((tcp_header->th_flags & (TH_SYN|TH_ACK)) == (TH_SYN|TH_ACK)) {
		// Both SYN and ACK flags are set - port is OPEN
		packet_received = 1;
		packet_type = 1;
	} else if (tcp_header->th_flags & TH_RST) {
		// RST flag is set - port is CLOSED
		packet_received = 1;
		packet_type = 2;
	}
}

/**
 * Craft and send a TCP SYN packet
 */
int send_syn_packet(int raw_socket, const char *src_ip, const char *dest_ip, uint16_t src_port, uint16_t dest_port) {
	char packet[100];
	memset(packet, 0, sizeof(packet));
	
	// IP header
	struct ip *ip_header = (struct ip *)packet;
	ip_header->ip_v = 4;
	ip_header->ip_hl = 5;
	ip_header->ip_tos = 0;
	ip_header->ip_len = htons(sizeof(struct ip) + sizeof(struct tcphdr));
	ip_header->ip_id = htons(rand() % 65535);
	ip_header->ip_off = 0;
	ip_header->ip_ttl = 64;
	ip_header->ip_p = IPPROTO_TCP;
	ip_header->ip_sum = 0;
	inet_pton(AF_INET, src_ip, &(ip_header->ip_src));
	inet_pton(AF_INET, dest_ip, &(ip_header->ip_dst));
	
	// Calculate IP header checksum
	ip_header->ip_sum = 0;
	
	// TCP header follows IP header
	struct tcphdr *tcp_header = (struct tcphdr *)(packet + sizeof(struct ip));
	tcp_header->th_sport = htons(src_port);
	tcp_header->th_dport = htons(dest_port);
	tcp_header->th_seq = htonl(rand() % 90000000 + 10000000);  // Random sequence number
	tcp_header->th_ack = 0;
	tcp_header->th_off = 5;
	tcp_header->th_flags = TH_SYN;
	tcp_header->th_win = htons(65535);
	tcp_header->th_sum = 0;
	tcp_header->th_urp = 0;
	
	// Calculate TCP checksum
	struct in_addr src, dst;
	inet_pton(AF_INET, src_ip, &src);
	inet_pton(AF_INET, dest_ip, &dst);
	tcp_header->th_sum = calculate_tcp_checksum(tcp_header, sizeof(struct tcphdr), src.s_addr, dst.s_addr);
	
	// Send the packet
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	inet_pton(AF_INET, dest_ip, &(dest_addr.sin_addr));
	
	int bytes_sent = sendto(raw_socket, packet, sizeof(struct ip) + sizeof(struct tcphdr), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	
	return bytes_sent;
}

/**
 * Get local IP address for the specified interface
 */
int get_local_ip(const char *interface, char *ip_buffer, size_t buffer_size) {
	struct ifaddrs *ifaddr, *ifa;
	int found = 0;
	int interface_exists = 0;

	// Try using getifaddrs which is more reliable
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		goto fallback;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_name && strcmp(ifa->ifa_name, interface) == 0) {
			interface_exists = 1;
			break;
		}
	}

	if (!interface_exists) {
		fprintf(stderr, "Interface %s does not exist\n", interface);
		freeifaddrs(ifaddr);
		strncpy(ip_buffer, "127.0.0.1", buffer_size);
		return 0;
	}

	// Now find the IPv4 address
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		
		if (strcmp(ifa->ifa_name, interface) == 0 && ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
			inet_ntop(AF_INET, &addr->sin_addr, ip_buffer, buffer_size);
			found = 1;
			break;
		}
	}

	freeifaddrs(ifaddr);

	if (found)
		return 0;
		
	// If we get here, the interface exists but has no IPv4 address
	fprintf(stderr, "No IPv4 address found for interface %s\n", interface);

	fallback:
	// Fallback to the ioctl - sometimes getifaddrs ipv4 doesn't work
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket failure in get_local_ip");
		strncpy(ip_buffer, "127.0.0.1", buffer_size);
		return 0;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl SIOCGIFADDR");
		close(fd);
		strncpy(ip_buffer, "127.0.0.1", buffer_size);
		return 0;
	}

	struct sockaddr_in *addr = (struct sockaddr_in*)&ifr.ifr_addr;
	snprintf(ip_buffer, buffer_size, "%s", inet_ntoa(addr->sin_addr));

	close(fd);
	return 0;
}

/**
 * Set up packet capture for TCP responses (non-blocking)
 */
pcap_t* setup_packet_capture(const char *interface, uint16_t port) {

	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter[100];
	bpf_u_int32 net, mask;
	
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
		return NULL;
	}
	
	// Create a filter to capture TCP packets FROM the specified port (responses)
	snprintf(filter, sizeof(filter), "tcp and src port %d and ((tcp[13] & 0x12) == 0x12 or (tcp[13] & 0x04) != 0)", port);
	
	// Compile and apply the filter
	if (pcap_compile(handle, &fp, filter, 0, net) == -1) {
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
 * TCP SYN scan implementation (non-blocking)
 */
port_status_t scan_tcp_port(const char *ip_addr, const char *interface, uint16_t port, int timeout) {
	int raw_socket;
	port_status_t status = PORT_FILTERED;
	char local_ip[INET_ADDRSTRLEN];
	struct timeval current_time, tv;
	long elapsed_time;
	fd_set read_fds;
	int pcap_fd;
	
	// Get local IP address for the interface
	if (get_local_ip(interface, local_ip, sizeof(local_ip)) < 0) {
		fprintf(stderr, "Failed to get local IP for interface %s\n", interface);
		return PORT_FILTERED;
	}
	
	// Create raw socket
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (raw_socket < 0) {
		perror("raw socket creation failed");
		return PORT_FILTERED;
	}
	
	// Set IP_HDRINCL option (we'll provide the IP header)
	int one = 1;
	if (setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
		perror("setsockopt IP_HDRINCL");
		close(raw_socket);
		return PORT_FILTERED;
	}
	
	// Set up packet capture
	pcap_handle = setup_packet_capture(interface, port);
	if (pcap_handle == NULL) {
		close(raw_socket);
		return PORT_FILTERED;
	}
	
	// Get pcap file descriptor for select()
	pcap_fd = pcap_get_selectable_fd(pcap_handle);
	if (pcap_fd < 0) {
		fprintf(stderr, "pcap handle doesn't support selectable file descriptor\n");
		pcap_close(pcap_handle);
		close(raw_socket);
		return PORT_FILTERED;
	}
	
	// Reset packet capture flags
	packet_received = 0;
	packet_type = 0;
	
	gettimeofday(&start_time, NULL);
	
	// Send SYN packet
	uint16_t src_port = 33000 + (rand() % 32000);  // Random source port
	if (send_syn_packet(raw_socket, local_ip, ip_addr, src_port, port) < 0) {
		perror("Failed to send SYN packet");
		pcap_close(pcap_handle);
		close(raw_socket);
		return PORT_FILTERED;
	}
	
	// Wait for response with timeout (non-blocking)
	while (1) {
		// Check if packet received
		if (packet_received) {
			if (packet_type == 1) {  // SYN-ACK received
				status = PORT_OPEN;
				break;
			} else if (packet_type == 2) {  // RST received
				status = PORT_CLOSED;
				break;
			}
		}
		
		// Check timeout
		gettimeofday(&current_time, NULL);
		elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000 + 
					(current_time.tv_usec - start_time.tv_usec) / 1000;
		
		if (elapsed_time >= timeout) {
			// No response after timeout, send another packet to verify
			packet_received = 0;
			packet_type = 0;
			
			// Update start time for the second attempt
			gettimeofday(&start_time, NULL);
			
			// Send another SYN packet with a different source port
			src_port = 33000 + (rand() % 32000);  // Random source port
			if (send_syn_packet(raw_socket, local_ip, ip_addr, src_port, port) < 0) {
				perror("Failed to send verification SYN packet");
				status = PORT_FILTERED;
				break;
			}
			
			// Wait for response to the second packet (non-blocking)
			int verification_wait = timeout / 2;  // Use half the timeout for verification
			long verification_start;
			gettimeofday(&current_time, NULL);
			verification_start = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
			
			while (1) {
				FD_ZERO(&read_fds);
				FD_SET(pcap_fd, &read_fds);
				
				// Calculate remaining time for select
				gettimeofday(&current_time, NULL);
				long current_ms = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
				long elapsed = current_ms - verification_start;
				
				if (elapsed >= verification_wait) {
					// Timeout
					break;
				}
				
				// Set timeout for select
				long remaining = verification_wait - elapsed;
				tv.tv_sec = remaining / 1000;
				tv.tv_usec = (remaining % 1000) * 1000;
				
				// Wait for data with select
				int select_result = select(pcap_fd + 1, &read_fds, NULL, NULL, &tv);
				
				if (select_result == -1) {
					perror("select");
					break;
				} else if (select_result == 0) {
					continue;
				}
				
				// Data available, process packet(s)
				if (FD_ISSET(pcap_fd, &read_fds)) {
					int rv = pcap_dispatch(pcap_handle, 1, packet_handler, NULL);
					if (rv < 0) {
						fprintf(stderr, "pcap_dispatch error: %s\n", pcap_geterr(pcap_handle));
						break;
					} else if (rv > 0 && packet_received) {
						break;
					}
				}
			}
			
			if (packet_received) {
				if (packet_type == 1) {  // SYN-ACK received
					status = PORT_OPEN;
				} else if (packet_type == 2) {  // RST received
					status = PORT_CLOSED;
				}
			} else {
				// Still no response, port is filtered
				status = PORT_FILTERED;
			}
			break;
		}
		
		// Set up for select() to wait for packets
		FD_ZERO(&read_fds);
		FD_SET(pcap_fd, &read_fds);
		
		// Set a short timeout for select
		tv.tv_sec = 0;
		tv.tv_usec = 10000;  // 10ms
		
		// Wait for data with select
		int select_result = select(pcap_fd + 1, &read_fds, NULL, NULL, &tv);
		
		if (select_result == -1) {
			perror("select");
			break;
		} else if (select_result == 0) {
			continue;
		}
		
		// Data available, process packet(s)
		if (FD_ISSET(pcap_fd, &read_fds)) {
			int rv = pcap_dispatch(pcap_handle, 1, packet_handler, NULL);
			if (rv < 0) {
				fprintf(stderr, "pcap_dispatch error: %s\n", pcap_geterr(pcap_handle));
				break;
			}
		}
	}
	
	// Final check - could occur if the last packet was received after the timeout
	struct pcap_stat stats;
	if (pcap_stats(pcap_handle, &stats) == 0) {
		if (stats.ps_recv > 0 && packet_received == 0) {
			fprintf(stderr, "Warning: Received %u packets but failed to process response\n", stats.ps_recv);
		}
	}

	// Clean up
	pcap_close(pcap_handle);
	close(raw_socket);
	
	return status;
}