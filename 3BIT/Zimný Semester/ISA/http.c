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
*              Implementation of the HTTP client for OTLP export               *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"

int parse_url(const char *url, char *host, int *port, char *path, int *use_https) {
	char *url_copy;
	char *scheme, *host_port;
	const char *path_part;
	char *port_str;
	
	if (!url || !host || !port || !path || !use_https) {
		return -1;
	}
	
	/* Default values */
	*port = 80;
	*use_https = 0;
	strcpy(path, "/");
	
	/* Create a copy of URL for parsing */
	url_copy = malloc(strlen(url) + 1);
	if (!url_copy) {
		return -1;
	}
	memcpy(url_copy, url, strlen(url) + 1);
	
	/* Parse scheme */
	scheme = strtok(url_copy, ":");
	if (!scheme) {
		free(url_copy);
		return -1;
	}
	
	if (strcmp(scheme, "https") == 0) {
		*use_https = 1;
		*port = 443;
	} else if (strcmp(scheme, "http") == 0) {
		*use_https = 0;
		*port = 80;
	} else {
		log_error("Unsupported URL scheme: %s\n", scheme);
		free(url_copy);
		return -1;
	}
	
	/* Skip "//" */
	host_port = strtok(NULL, "/");
	if (!host_port || strncmp(url + strlen(scheme) + 1, "//", 2) != 0) {
		free(url_copy);
		return -1;
	}
	
	/* Parse host and port */
	port_str = strchr(host_port, ':');
	if (port_str) {
		*port_str = '\0';
		port_str++;
		*port = atoi(port_str);
		if (*port <= 0 || *port > 65535) {
			log_error("Invalid port number: %s\n", port_str);
			free(url_copy);
			return -1;
		}
	}
	
	strncpy(host, host_port, MAX_HOSTNAME_LEN - 1);
	host[MAX_HOSTNAME_LEN - 1] = '\0';
	
	/* Parse path */
	path_part = strtok(NULL, "");
	if (path_part) {
		snprintf(path, MAX_URL_LEN, "/%s", path_part);
	}
	
	free(url_copy);
	return 0;
}

int send_http_request(const char *host, int port, const char *path, const char *data, int use_https) {
	int sockfd;
	struct sockaddr_in server_addr;
	char request[MAX_BUFFER_SIZE];
	char response[MAX_BUFFER_SIZE];
	int request_len, response_len;
	char *response_body;
	int content_length;
	
	if (use_https) {
		log_error("HTTPS not supported in this implementation\n");
		return -1;
	}
	
	/* Create TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		log_error("Failed to create socket: %s\n", strerror(errno));
		return -1;
	}
	
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0 || setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		log_error("Failed to set socket timeout: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	
	/* Resolve hostname and connect */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t) port);
	
	if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
		struct addrinfo hints, *result;
		
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		
		if (getaddrinfo(host, NULL, &hints, &result) != 0) {
			log_error("Failed to resolve hostname: %s\n", host);
			close(sockfd);
			return -1;
		}
		
		memcpy(&server_addr, result->ai_addr, sizeof(server_addr));
		server_addr.sin_port = htons((uint16_t) port);
		freeaddrinfo(result);
	}
	
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		log_error("Failed to connect to %s:%d: %s\n", host, port, strerror(errno));
		close(sockfd);
		return -1;
	}
	
	log_message(verbose_mode, "Connected to %s:%d\n", host, port);
	
	/* Build HTTP request */
	content_length = (int) strlen(data);
	request_len = snprintf(request, sizeof(request),
		"POST %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"User-Agent: snmp2otel/1.0\r\n"
		"Connection: close\r\n"
		"\r\n"
		"%s",
		path, host, port, content_length, data);
	
	if (request_len >= (int) sizeof(request)) {
		log_error("HTTP request too large\n");
		close(sockfd);
		return -1;
	}
	
	log_message(verbose_mode, "Sending HTTP request (%d bytes)\n", request_len);
	
	/* Send request */
	if (send(sockfd, request, (size_t) request_len, 0) != request_len) {
		log_error("Failed to send HTTP request: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	
	/* Receive response */
	response_len = (int) recv(sockfd, response, sizeof(response) - 1, 0);
	if (response_len < 0) {
		log_error("Failed to receive HTTP response: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	
	response[response_len] = '\0';
	close(sockfd);
	
	log_message(verbose_mode, "Received HTTP response (%d bytes)\n", response_len);
	
	/* Parse HTTP response */
	const char *status_line = strtok(response, "\r\n");
	if (!status_line) {
		log_error("Invalid HTTP response: no status line\n");
		return -1;
	}
	
	/* Check status code */
	const char *status_code_str = strchr(status_line, ' ');
	if (!status_code_str) {
		log_error("Invalid HTTP response: malformed status line\n");
		return -1;
	}
	status_code_str++;
	
	int status_code = atoi(status_code_str);
	log_message(verbose_mode, "HTTP response status: %d\n", status_code);
	
	if (status_code < 200 || status_code >= 300) {
		log_error("HTTP request failed with status %d\n", status_code);
		
		response_body = strstr(response, "\r\n\r\n");
		if (response_body) {
			response_body += 4;
			log_error("Response body: %s\n", response_body);
		}
		
		return -1;
	}
	
	/* Find response body */
	response_body = strstr(response, "\r\n\r\n");
	if (response_body) {
		response_body += 4;
		log_message(verbose_mode, "Response body: %s\n", response_body);
	}
	
	return 0;
}