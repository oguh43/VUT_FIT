/**
 * xbohach00
 * tcp_client.c - TCP protocol variant implementation
 */
#include "tcp_client.h"

// Buffer for network data
static char recv_buffer[MAX_BUFFER_SIZE];
static char send_buffer[MAX_BUFFER_SIZE];

// TCP message buffer (can span multiple packets)
static char tcp_message_buffer[MAX_BUFFER_SIZE*2];
static size_t tcp_buffer_used = 0;

// Initialize TCP client
bool tcp_client_init(const char *host, int port) {
	struct hostent *server;
	struct sockaddr_in server_addr;
	
	// Get host information
	server = gethostbyname(host);
	if (server == NULL) {
		print_error("Could not resolve hostname: %s", host);
		return false;
	}
	
	// Create socket
	config.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (config.socket_fd < 0) {
		print_error("Could not create socket: %s", strerror(errno));
		return false;
	}
	
	// Prepare server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	server_addr.sin_port = htons(port);
	
	// Connect to server
	if (connect(config.socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		print_error("Could not connect to server: %s", strerror(errno));
		close(config.socket_fd);
		return false;
	}
	
	// Initialize state
	config.state = STATE_START;
	
	printf_debug("TCP client connected to %s:%d", host, port);
	return true;
}

// Process and handle a received TCP message
static void tcp_process_message(const char *message) {
	MessageResult result;
	ResponseType response_type = tcp_parse_message(message, &result);
	
	// Debug output
	printf_debug("Received message: '%s'", message);
	printf_debug("Parsed as type: %d", response_type);
	
	switch (response_type) {
		case RESPONSE_SUCCESS:
			printf("Action Success: %s\n", result.content);
			
			// Update state
			if (config.state == STATE_AUTH) {
				config.state = STATE_OPEN;
			}
			break;
			
		case RESPONSE_FAILURE:
			printf("Action Failure: %s\n", result.content);
			break;
			
		case RESPONSE_MSG:
			printf("%s: %s\n", result.display_name, result.content);
			break;
			
		case RESPONSE_ERROR:
			printf("ERROR FROM %s: %s\n", result.display_name, result.content);
			config.running = false;  // Terminate on error
			break;
			
		case RESPONSE_BYE:
			printf_debug("Received BYE from %s", result.display_name);
			config.running = false;  // Terminate on BYE
			break;
			
		case RESPONSE_NONE:
			print_error("Received malformed message from server: '%s'", message);
			// Send ERR message and terminate
			snprintf(send_buffer, sizeof(send_buffer), "ERR FROM %s IS Malformed message received\r\n", config.display_name);
			send(config.socket_fd, send_buffer, strlen(send_buffer), 0);
			config.running = false;
			break;
			
		default:
			break;
	}
}

// Reset TCP buffer
static void tcp_buffer_reset(void) {
	memset(tcp_message_buffer, 0, sizeof(tcp_message_buffer));
	tcp_buffer_used = 0;
}

// Try to extract complete TCP messages from buffer
static void tcp_process_received_data(void) {
	// Debug
	printf_debug("Processing buffer with %zu bytes", tcp_buffer_used);
	
	if (tcp_buffer_used < sizeof(tcp_message_buffer)) {
		tcp_message_buffer[tcp_buffer_used] = '\0';
	} else {
		tcp_message_buffer[sizeof(tcp_message_buffer) - 1] = '\0';
	}
	
	// Process buffer one character at a time
	size_t processed = 0;
	while (processed < tcp_buffer_used) {
		char *crlf_pos = NULL;
		for (size_t i = processed; i < tcp_buffer_used - 1; i++) {
			if (tcp_message_buffer[i] == '\r' && tcp_message_buffer[i + 1] == '\n') {
				crlf_pos = &tcp_message_buffer[i];
				break;
			}
		}
		
		// If no CRLF found, we're done processing
		if (crlf_pos == NULL) {
			break;
		}
		
		// Message length
		size_t message_len = crlf_pos - &tcp_message_buffer[processed];
		
		// Copy message to a temporary buffer
		char temp_message[MAX_BUFFER_SIZE];
		if (message_len >= MAX_BUFFER_SIZE) {
			message_len = MAX_BUFFER_SIZE - 1;
		}
		memcpy(temp_message, &tcp_message_buffer[processed], message_len);
		temp_message[message_len] = '\0';
		
		printf_debug("Extracted message: '%s'", temp_message);
		
		// Process the complete message
		tcp_process_message(temp_message);
		
		processed += message_len + 2;
	}
	
	// If we processed anything, remove it from the buffer
	if (processed > 0) {
		if (processed < tcp_buffer_used) {
			memmove(tcp_message_buffer, &tcp_message_buffer[processed], 
					tcp_buffer_used - processed);
			tcp_buffer_used -= processed;
		} else {
			tcp_buffer_reset();
		}
	}
	
	// Buffer overflow protection
	if (tcp_buffer_used > MAX_BUFFER_SIZE * 0.75) {
		printf_debug("Warning: TCP buffer is getting full (%zu bytes)", tcp_buffer_used);
		
		// Print some of the buffer content for debugging
		size_t print_len = tcp_buffer_used < 100 ? tcp_buffer_used : 100;
		
		// Create a hex dump
		char hex_dump[300] = {0};
		char *ptr = hex_dump;
		for (size_t i = 0; i < print_len && i < 50; i++) {
			ptr += sprintf(ptr, "%02x ", (unsigned char)tcp_message_buffer[i]);
		}
		printf_debug("HEX: %s", hex_dump);
		
		// Create an ASCII dump
		char ascii_dump[150] = {0};
		ptr = ascii_dump;
		for (size_t i = 0; i < print_len && i < 50; i++) {
			char c = tcp_message_buffer[i];
			ptr += sprintf(ptr, "%c", (c >= 32 && c <= 126) ? c : '.');
		}
		printf_debug("ASCII: %s", ascii_dump);
	}
	
	if (tcp_buffer_used >= MAX_BUFFER_SIZE * 0.9) {
		printf_debug("TCP buffer overflow, clearing buffer");
		tcp_buffer_reset();
	}
}

// Process user command in TCP mode
bool tcp_client_process_command(const Command *cmd) {
	if (!cmd) {
		return false;
	}
	
	int bytes_to_send = 0;
	
	switch (cmd->type) {
		case CMD_AUTH:
			bytes_to_send = tcp_create_auth_message(send_buffer, sizeof(send_buffer), 
												   cmd->username, cmd->display_name, cmd->secret);
			
			// Store the display name in config
			strncpy(config.display_name, cmd->display_name, MAX_DISPLAYNAME_LENGTH);
			
			config.state = STATE_AUTH;
			break;
			
		case CMD_JOIN:
			bytes_to_send = tcp_create_join_message(send_buffer, sizeof(send_buffer), 
												   cmd->channel_id, config.display_name);
			break;
			
		case CMD_RENAME:
			// Just update the display name locally
			strncpy(config.display_name, cmd->display_name, MAX_DISPLAYNAME_LENGTH);
			printf_debug("Display name changed to: %s", config.display_name);
			return true;
			
		case CMD_HELP:
			display_help();
			return true;
			
		case CMD_MESSAGE:
			// Check if message is too long and truncate if necessary
			if (strlen(cmd->message) > MAX_MESSAGE_CONTENT_LENGTH) {
				print_error("Message too long, truncating to %d characters", MAX_MESSAGE_CONTENT_LENGTH);
				char truncated_message[MAX_MESSAGE_CONTENT_LENGTH + 1];
				strncpy(truncated_message, cmd->message, MAX_MESSAGE_CONTENT_LENGTH);
				truncated_message[MAX_MESSAGE_CONTENT_LENGTH] = '\0';
				
				bytes_to_send = tcp_create_msg_message(send_buffer, sizeof(send_buffer), config.display_name, truncated_message);
			} else {
				bytes_to_send = tcp_create_msg_message(send_buffer, sizeof(send_buffer), config.display_name, cmd->message);
			}
			break;
			
		default:
			print_error("Unknown command type");
			return false;
	}
	
	if (bytes_to_send <= 0) {
		print_error("Failed to create message");
		return false;
	}
	
	// Send the message
	if (send(config.socket_fd, send_buffer, bytes_to_send, 0) != bytes_to_send) {
		print_error("Failed to send message: %s", strerror(errno));
		return false;
	}
	
	printf_debug("Sent message: %.*s", bytes_to_send, send_buffer);
	return true;
}

// Run the TCP client main loop
void tcp_client_run(void) {
	fd_set read_fds;
	struct timeval tv;
	
	config.running = true;
	
	while (config.running && !termination_requested) {
		// Prepare for select
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		FD_SET(config.socket_fd, &read_fds);
		
		// Set timeout
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		// Wait for input or socket data
		int select_result = select(config.socket_fd + 1, &read_fds, NULL, NULL, &tv);
		
		if (select_result < 0) {
			if (errno == EINTR) {
				// Interrupted by signal, check if termination was requested
				continue;
			}
			print_error("Select failed: %s", strerror(errno));
			break;
		}
		
		// Check for termination request
		if (termination_requested) {
			printf_debug("Termination requested, sending BYE message");
			if (config.state == STATE_OPEN) {
				// Send BYE message
				int bytes_to_send = tcp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name);
				if (bytes_to_send > 0) {
					send(config.socket_fd, send_buffer, bytes_to_send, 0);
					printf_debug("BYE message sent");
				}
			}
			break;
		}
		
		// Check if socket has data
		if (FD_ISSET(config.socket_fd, &read_fds)) {
			ssize_t bytes_received = recv(config.socket_fd, recv_buffer, sizeof(recv_buffer) - 1, 0);
			
			if (bytes_received < 0) {
				print_error("Error receiving data: %s", strerror(errno));
				break;
			} else if (bytes_received == 0) {
				// Connection closed by server
				printf_debug("Connection closed by server");
				break;
			} else {
				recv_buffer[bytes_received] = '\0';
				
				// Check if we have enough space in the message buffer
				if (tcp_buffer_used + bytes_received > sizeof(tcp_message_buffer)) {
					print_error("Message buffer overflow");
					break;
				}
				
				// Append received data to the message buffer
				memcpy(tcp_message_buffer + tcp_buffer_used, recv_buffer, bytes_received);
				tcp_buffer_used += bytes_received;
				
				// Debug output
				printf_debug("Received %zd bytes, buffer now has %zu bytes", bytes_received, tcp_buffer_used);
				
				tcp_process_received_data();
			}
		}
		
		// Check if stdin has data
		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			char input_line[MAX_BUFFER_SIZE] = {0};
			
			if (fgets(input_line, sizeof(input_line), stdin) != NULL) {
				size_t len = strlen(input_line);
				if (len > 0 && input_line[len - 1] == '\n') {
					input_line[len - 1] = '\0';
				}
				
				// Process the input
				Command cmd;
				if (process_input_line(input_line, &cmd)) {
					if (is_command_valid(&cmd, config.state)) {
						tcp_client_process_command(&cmd);
					} else {
						print_error("Command not valid in current state");
					}
				}
			} else {
				// EOF or error on stdin, terminate gracefully
				printf_debug("End of input, terminating gracefully");
				if (config.state == STATE_OPEN) {
					// Send BYE message
					int bytes_to_send = tcp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name);
					if (bytes_to_send > 0) {
						send(config.socket_fd, send_buffer, bytes_to_send, 0);
						printf_debug("BYE message sent");
					}
				}
				
				config.running = false;
			}
		}
	}
	
	// If in OPEN state and normal termination, send BYE message
	if (config.state == STATE_OPEN && config.running) {
		int bytes_to_send = tcp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name);
		if (bytes_to_send > 0) {
			send(config.socket_fd, send_buffer, bytes_to_send, 0);
			printf_debug("BYE message sent");
		}
	}
}

// Clean up TCP client resources
void tcp_client_cleanup(void) {
	if (config.socket_fd != -1) {
		close(config.socket_fd);
		config.socket_fd = -1;
	}
}