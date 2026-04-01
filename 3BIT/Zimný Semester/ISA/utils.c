/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*               Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)             *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*             Implementation of the Utility functions for snmp2otel            *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"
#include <stdarg.h>

int verbose_mode = 0;

void log_message(int is_verbose, const char *format, ...) {
	
	if (is_verbose && verbose_mode) {
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format, args);
		va_end(args);
		fflush(stdout);
	}
}

void log_error(const char *format, ...) {
	va_list args;
	
	va_start(args, format);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}

unsigned long get_timestamp_ms(void) {
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) != 0) {
		return 0;
	}
	
	return (unsigned long)((long unsigned int) tv.tv_sec * 1000UL + (long unsigned int) tv.tv_usec / 1000UL);
}

int resolve_hostname(const char *hostname, char *ip_str) {
	struct in_addr addr;
	
	if (!hostname || !ip_str) {
		return -1;
	}
	
	/* Check if it's already an IP address */
	if (inet_pton(AF_INET, hostname, &addr)) {
		strcpy(ip_str, hostname);
		return 0;
	}
	
	/* Try to resolve hostname using getaddrinfo */
	struct addrinfo hints, *result;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
		return -1;
	}

	/* Extract IP address */
	struct sockaddr_in *addr_in = (struct sockaddr_in *)result->ai_addr;
	strcpy(ip_str, inet_ntoa(addr_in->sin_addr));
	freeaddrinfo(result);

	return 0;
}