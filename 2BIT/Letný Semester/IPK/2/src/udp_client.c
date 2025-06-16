/**
 * xbohach00
 * udp_client.c - UDP protocol variant implementation
 */
#include "udp_client.h"

// Network data buffers
static uint8_t recv_buffer[MAX_BUFFER_SIZE];
static uint8_t send_buffer[MAX_BUFFER_SIZE];

// Variables for handling message retransmission
static bool waiting_for_confirm = false;
static uint16_t waiting_for_msg_id = 0;
static struct timespec confirm_wait_start;
static uint8_t last_message[MAX_BUFFER_SIZE];
static size_t last_message_size = 0;
static int retries_left = 0;

static bool waiting_for_reply = false;
static uint16_t waiting_for_reply_to_msg_id = 0;
static struct timespec reply_wait_start;

// Set of received message IDs to detect duplicates
static uint16_t seen_message_ids[256][64];
static size_t seen_message_count[256] = {0};

// Initialize UDP client
bool udp_client_init(const char *host, int port, int timeout_ms, int max_retries) {
	struct hostent *server;
	struct sockaddr_in server_addr;
	
	// Get host information
	server = gethostbyname(host);
	if (server == NULL) {
		print_error("Could not resolve hostname: %s", host);
		return false;
	}
	
	// Create socket
	config.socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (config.socket_fd < 0) {
		print_error("Could not create socket: %s", strerror(errno));
		return false;
	}
	
	// Prepare server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	server_addr.sin_port = htons(port);
	memcpy(&config.server_dyn_addr, &server_addr, sizeof(server_addr));
	
	// Initialize state and counters
	config.state = STATE_START;
	config.next_message_id = 0;
	config.timeout_ms = timeout_ms;
	config.max_retries = max_retries;
	
	printf_debug("UDP client initialized for %s:%d (timeout: %dms, retries: %d)", host, port, timeout_ms, max_retries);
	return true;
}

// Helper: Check if a message ID has been seen before for a specific message type
static bool has_seen_message_id(uint8_t msg_type, uint16_t msg_id) {
	if (msg_type == MSG_TYPE_CONFIRM) {
		return false;
	}

	// Look for the message ID in the type-specific array
	for (size_t i = 0; i < seen_message_count[msg_type]; i++) {
		if (seen_message_ids[msg_type][i] == msg_id) {
			return true;
		}
	}
	
	// Add to seen list if not found
	if (seen_message_count[msg_type] < 64) {
		seen_message_ids[msg_type][seen_message_count[msg_type]++] = msg_id;
	} else {
		memmove(seen_message_ids[msg_type], seen_message_ids[msg_type] + 1, (64 - 1) * sizeof(uint16_t));
		seen_message_ids[msg_type][64 - 1] = msg_id;
	}
	
	return false;
}

// Helper: Get current time
static void get_current_time(struct timespec *ts) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;
}

// Helper: Calculate time difference in milliseconds
static long time_diff_ms(struct timespec *start, struct timespec *end) {
	return (end->tv_sec - start->tv_sec) * 1000 + 
		   (end->tv_nsec - start->tv_nsec) / 1000000;
}

// Send a message to the server and wait for confirmation
static bool udp_send_message(const uint8_t *data, size_t data_len, bool is_confirm) {
	struct sockaddr_in *target_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	// Determine target address (initial server or dynamic port)
	if (config.server_dyn_port == 0) {
		target_addr = &config.server_dyn_addr;
	} else {
		target_addr = &config.server_dyn_addr;
		target_addr->sin_port = htons(config.server_dyn_port);
	}
	
	// Send the message
	if (sendto(config.socket_fd, data, data_len, 0, (struct sockaddr *)target_addr, addr_len) != (ssize_t)data_len) {
		print_error("Failed to send message: %s", strerror(errno));
		return false;
	}
	
	printf_debug("Sent %zu bytes to %s:%d", data_len, inet_ntoa(target_addr->sin_addr), ntohs(target_addr->sin_port));
	
	if (is_confirm) {
		return true;
	}
	
	// For other messages, we need to wait for confirmation
	waiting_for_confirm = true;
	waiting_for_msg_id = (data[1] << 8) | data[2];
	get_current_time(&confirm_wait_start);
	
	// Save message for potential retransmission
	memcpy(last_message, data, data_len);
	last_message_size = data_len;
	retries_left = config.max_retries;
	
	return true;
}

// Process and handle a received UDP message
static void udp_process_message(const uint8_t *message, size_t message_len, struct sockaddr_in *sender_addr) {
	if (message_len < 3) {
		print_error("Received malformed message (too short)");
		return;
	}
	
	MessageResult result;
	uint16_t msg_id = 0;
	uint16_t ref_msg_id = 0;
	
	ResponseType response_type = udp_parse_message(message, message_len, &result, &msg_id, &ref_msg_id);
	
	// Check for duplicate messages
	if (message[0] != MSG_TYPE_CONFIRM && has_seen_message_id(message[0], msg_id)) {
		printf_debug("Received duplicate message type %d with ID %d, sending confirmation", message[0], msg_id);
		
		// Send confirmation for the duplicate message
		int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
		if (confirm_len > 0) {
			udp_send_message(send_buffer, confirm_len, true);
		}
		return;
	}
	
	// Update dynamic server port if this is the first response
	if (config.server_dyn_port == 0 && sender_addr) {
		config.server_dyn_port = ntohs(sender_addr->sin_port);
		printf_debug("Updated server dynamic port to %d", config.server_dyn_port);
		config.server_dyn_addr.sin_port = htons(config.server_dyn_port);
	}
	
	// If port changes, update it
	if (sender_addr && ntohs(sender_addr->sin_port) != config.server_dyn_port) {
		printf_debug("Server port changed from %d to %d", config.server_dyn_port, ntohs(sender_addr->sin_port));
		config.server_dyn_port = ntohs(sender_addr->sin_port);
		config.server_dyn_addr.sin_port = htons(config.server_dyn_port);
	}
	
	// Process based on message type
	switch (response_type) {
		case RESPONSE_CONFIRM:
			if (waiting_for_confirm && ref_msg_id == waiting_for_msg_id) {
				printf_debug("Received confirmation for message %d", ref_msg_id);
				waiting_for_confirm = false;
			} else {
				printf_debug("Received unexpected confirmation for message %d", ref_msg_id);
			}
			break;
			
		case RESPONSE_SUCCESS:
			// Send confirmation
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			printf("Action Success: %s\n", result.content);
			
			// Check if this is a reply to our request
			if (waiting_for_reply && ref_msg_id == waiting_for_reply_to_msg_id) {
				waiting_for_reply = false;
				
				// Update FSM state
				if (config.state == STATE_AUTH) {
					config.state = STATE_OPEN;
				}
			}
			break;
			
		case RESPONSE_FAILURE:
			// Send confirmation
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			printf("Action Failure: %s\n", result.content);
			
			// Check if this is a reply to our request
			if (waiting_for_reply && ref_msg_id == waiting_for_reply_to_msg_id) {
				waiting_for_reply = false;
			}
			break;
			
		case RESPONSE_MSG:
			// Send confirmation
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			printf("%s: %s\n", result.display_name, result.content);
			break;
			
		case RESPONSE_ERROR:
			// Send confirmation
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			// Handle error message
			printf("ERROR FROM %s: %s\n", result.display_name, result.content);
			config.running = false;  // Terminate on error
			break;
			
		case RESPONSE_BYE:
			// Send confirmation
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			printf_debug("Received BYE from %s", result.display_name);
			config.running = false;  // Terminate on BYE
			break;
			
		case RESPONSE_PING:
			// Send confirmation for PING
			if (message[0] != MSG_TYPE_CONFIRM) {
				int confirm_len = udp_create_confirm_message(send_buffer, sizeof(send_buffer), msg_id);
				if (confirm_len > 0) {
					udp_send_message(send_buffer, confirm_len, true);
				}
			}
			
			printf_debug("Received PING from server");
			break;
			
		case RESPONSE_NONE:
			print_error("Received malformed message from server");
			// Send ERR message and terminate
			int err_len = udp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name, config.next_message_id++);
			if (err_len > 0) {
				udp_send_message(send_buffer, err_len, false);
			}
			config.running = false;
			break;
	}
}

// Process user command in UDP mode
bool udp_client_process_command(const Command *cmd) {
	if (!cmd) {
		return false;
	}
	
	// Don't process new commands if we're waiting for confirmation
	if (waiting_for_confirm) {
		print_error("Waiting for server confirmation, please wait");
		return false;
	}
	
	int bytes_to_send = 0;
	
	switch (cmd->type) {
		case CMD_AUTH:
			bytes_to_send = udp_create_auth_message(send_buffer, sizeof(send_buffer), cmd->username, cmd->display_name, cmd->secret,config.next_message_id);
			
			strncpy(config.display_name, cmd->display_name, MAX_DISPLAYNAME_LENGTH);
			
			// Set waiting for reply
			waiting_for_reply = true;
			waiting_for_reply_to_msg_id = config.next_message_id;
			get_current_time(&reply_wait_start);
			
			// Update FSM and message ID
			config.state = STATE_AUTH;
			config.next_message_id++;
			break;
			
		case CMD_JOIN:
			bytes_to_send = udp_create_join_message(send_buffer, sizeof(send_buffer), cmd->channel_id, config.display_name, config.next_message_id);
			
			// Set waiting for reply
			waiting_for_reply = true;
			waiting_for_reply_to_msg_id = config.next_message_id;
			get_current_time(&reply_wait_start);
			
			// Update message ID
			config.next_message_id++;
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
				
				bytes_to_send = udp_create_msg_message(send_buffer, sizeof(send_buffer), config.display_name, truncated_message, config.next_message_id);
			} else {
				bytes_to_send = udp_create_msg_message(send_buffer, sizeof(send_buffer), config.display_name, cmd->message, config.next_message_id);
			}
			
			// Update message ID
			config.next_message_id++;
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
	return udp_send_message(send_buffer, bytes_to_send, false);
}

// Run the UDP client main loop
void udp_client_run(void) {
	fd_set read_fds;
	struct timeval tv;
	struct sockaddr_in sender_addr;
	socklen_t sender_addr_len = sizeof(sender_addr);
	struct timespec current_time;
	
	config.running = true;
	
	while (config.running) {
		// Check for termination request
		if (termination_requested) {
			printf_debug("Termination requested, sending BYE message");
			if (config.state == STATE_OPEN && !waiting_for_confirm) {
				// Send BYE message
				int bytes_to_send = udp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name, config.next_message_id++);
				if (bytes_to_send > 0) {
					udp_send_message(send_buffer, bytes_to_send, false);
					printf_debug("BYE message sent, waiting for confirmation");
					
					// Wait for confirmation with shorter timeouts
					int confirm_wait_count = 10;
					while (waiting_for_confirm && confirm_wait_count > 0) {
						fd_set temp_fds;
						struct timeval temp_tv;
						
						FD_ZERO(&temp_fds);
						FD_SET(config.socket_fd, &temp_fds);
						
						temp_tv.tv_sec = 0;
						temp_tv.tv_usec = 100000; // 100ms
						
						if (select(config.socket_fd + 1, &temp_fds, NULL, NULL, &temp_tv) > 0) {
							if (FD_ISSET(config.socket_fd, &temp_fds)) {
								ssize_t bytes_received = recvfrom(config.socket_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
								
								if (bytes_received > 0) {
									udp_process_message(recv_buffer, bytes_received, &sender_addr);
								}
							}
						}
						
						confirm_wait_count--;
					}
					
					if (waiting_for_confirm) {
						printf_debug("BYE confirmation not received, giving up");
					} else {
						printf_debug("BYE confirmation received, terminating");
					}
				}
			}
			break;
		}
		
		// Check for timeout if waiting for confirmation
		if (waiting_for_confirm) {
			get_current_time(&current_time);
			long elapsed_ms = time_diff_ms(&confirm_wait_start, &current_time);
			
			if (elapsed_ms >= config.timeout_ms) {
				if (retries_left > 0) {
					printf_debug("Confirmation timeout, retrying (%d retries left)", retries_left);
					
					// Resend the message
					if (sendto(config.socket_fd, last_message, last_message_size, 0, (struct sockaddr *)&config.server_dyn_addr, sizeof(config.server_dyn_addr)) != (ssize_t)last_message_size) {
						print_error("Failed to resend message: %s", strerror(errno));
					}
					
					// Update timeout start time and decrease retries
					get_current_time(&confirm_wait_start);
					retries_left--;
				} else {
					print_error("Maximum retries reached, connection failed");
					config.running = false;
					break;
				}
			}
		}
		
		// Check for REPLY timeout
		if (waiting_for_reply) {
			get_current_time(&current_time);
			long elapsed_ms = time_diff_ms(&reply_wait_start, &current_time);
			
			if (elapsed_ms >= 5000) {
				print_error("Timeout waiting for server reply");
				
				// Send BYE message
				if (config.state == STATE_OPEN) {
					int err_len = udp_create_bye_message(send_buffer, sizeof(send_buffer), 
													   config.display_name, 
													   config.next_message_id++);
					if (err_len > 0) {
						udp_send_message(send_buffer, err_len, false);
					}
				}
				
				config.running = false;
				break;
			}
		}
		
		// Prepare for select
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);  // Add stdin
		FD_SET(config.socket_fd, &read_fds);  // Add socket
		
		if (waiting_for_confirm) {
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
		} else {
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
		}
		
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
		
		// Check if socket has data
		if (FD_ISSET(config.socket_fd, &read_fds)) {
			ssize_t bytes_received = recvfrom(config.socket_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
			
			if (bytes_received < 0) {
				print_error("Error receiving data: %s", strerror(errno));
				break;
			} else if (bytes_received > 0) {
				printf_debug("Received %zd bytes from %s:%d", bytes_received, inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
				
				// Process the message
				udp_process_message(recv_buffer, bytes_received, &sender_addr);
			}
		}
		
		// Check if stdin has data and we're not waiting for confirmation
		if (FD_ISSET(STDIN_FILENO, &read_fds) && !waiting_for_confirm) {
			char input_line[MAX_BUFFER_SIZE] = {0};
			
			if (fgets(input_line, sizeof(input_line), stdin) != NULL) {
				// Remove trailing newline
				size_t len = strlen(input_line);
				if (len > 0 && input_line[len - 1] == '\n') {
					input_line[len - 1] = '\0';
				}
				
				// Process the input
				Command cmd;
				if (process_input_line(input_line, &cmd)) {
					if (is_command_valid(&cmd, config.state)) {
						udp_client_process_command(&cmd);
					} else {
						print_error("Command not valid in current state");
					}
				}
			} else {
				// EOF or error on stdin, terminate gracefully
				printf_debug("End of input, terminating gracefully");
				if (config.state == STATE_OPEN) {
					// Send BYE message
					int bytes_to_send = udp_create_bye_message(send_buffer, sizeof(send_buffer), config.display_name, config.next_message_id++);
					if (bytes_to_send > 0) {
						udp_send_message(send_buffer, bytes_to_send, false);
						printf_debug("BYE message sent");
					}
				}
				
				config.running = false;
			}
		}
	}
}

// Clean up UDP client resources
void udp_client_cleanup(void) {
	if (config.socket_fd != -1) {
		close(config.socket_fd);
		config.socket_fd = -1;
	}
}