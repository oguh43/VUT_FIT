/**
 * xbohach00
 * message.c - Message handling implementation
 */
#include "message.h"
#include <stdarg.h>

// Print formatted error to stdout
void print_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	printf("ERROR: ");
	vprintf(format, args);
	printf("\n");
	
	va_end(args);
}

// Print formatted error to stdout and exit
void exit_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	printf("ERROR: ");
	vprintf(format, args);
	printf("\n");
	
	va_end(args);
	exit(EXIT_FAILURE);
}

/* TCP Message Creation Functions */

int tcp_create_auth_message(char *buffer, size_t buffer_size, const char *username, const char *display_name, const char *secret) {
	return snprintf(buffer, buffer_size, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);
}

int tcp_create_join_message(char *buffer, size_t buffer_size, const char *channel_id, const char *display_name) {
	return snprintf(buffer, buffer_size, "JOIN %s AS %s\r\n", channel_id, display_name);
}

int tcp_create_msg_message(char *buffer, size_t buffer_size, const char *display_name, const char *content) {
	return snprintf(buffer, buffer_size, "MSG FROM %s IS %s\r\n", display_name, content);
}

int tcp_create_bye_message(char *buffer, size_t buffer_size, const char *display_name) {
	return snprintf(buffer, buffer_size, "BYE FROM %s\r\n", display_name);
}

/* TCP Message Parsing Function */

ResponseType tcp_parse_message(const char *message, MessageResult *result) {
	// Reset result structure
	memset(result, 0, sizeof(MessageResult));
	
	// Check message type
	if (strncmp(message, "REPLY OK IS ", 12) == 0) {
		strncpy(result->content, message + 12, MAX_MESSAGE_CONTENT_LENGTH);
		result->content[MAX_MESSAGE_CONTENT_LENGTH] = '\0';
		result->type = RESPONSE_SUCCESS;
		return RESPONSE_SUCCESS;
	} else if (strncmp(message, "REPLY NOK IS ", 13) == 0) {
		strncpy(result->content, message + 13, MAX_MESSAGE_CONTENT_LENGTH);
		result->content[MAX_MESSAGE_CONTENT_LENGTH] = '\0';
		result->type = RESPONSE_FAILURE;
		return RESPONSE_FAILURE;
	} else if (strncmp(message, "MSG FROM ", 9) == 0) {
		const char *display_name_start = message + 9;
		const char *is_marker = strstr(display_name_start, " IS ");
		
		if (is_marker) {
			size_t display_name_len = is_marker - display_name_start;
			if (display_name_len > MAX_DISPLAYNAME_LENGTH) {
				display_name_len = MAX_DISPLAYNAME_LENGTH;
			}
			strncpy(result->display_name, display_name_start, display_name_len);
			result->display_name[display_name_len] = '\0';
			
			strncpy(result->content, is_marker + 4, MAX_MESSAGE_CONTENT_LENGTH);
			result->content[MAX_MESSAGE_CONTENT_LENGTH] = '\0';
			result->type = RESPONSE_MSG;
			return RESPONSE_MSG;
		}
	} else if (strncmp(message, "ERR FROM ", 9) == 0) {
		const char *display_name_start = message + 9;
		const char *is_marker = strstr(display_name_start, " IS ");
		
		if (is_marker) {
			size_t display_name_len = is_marker - display_name_start;
			if (display_name_len > MAX_DISPLAYNAME_LENGTH) {
				display_name_len = MAX_DISPLAYNAME_LENGTH;
			}
			strncpy(result->display_name, display_name_start, display_name_len);
			result->display_name[display_name_len] = '\0';
			
			strncpy(result->content, is_marker + 4, MAX_MESSAGE_CONTENT_LENGTH);
			result->content[MAX_MESSAGE_CONTENT_LENGTH] = '\0';
			result->type = RESPONSE_ERROR;
			return RESPONSE_ERROR;
		}
	} else if (strncmp(message, "BYE FROM ", 9) == 0) {
		strncpy(result->display_name, message + 9, MAX_DISPLAYNAME_LENGTH);
		result->display_name[MAX_DISPLAYNAME_LENGTH] = '\0';
		result->type = RESPONSE_BYE;
		return RESPONSE_BYE;
	}
	
	printf_debug("Failed to parse message: '%s'", message);
	result->type = RESPONSE_NONE;
	return RESPONSE_NONE;
}

/* UDP Message Creation Functions */

// Helper for writing zero-terminated string to buffer
static int write_stringz(uint8_t *buffer, size_t offset, size_t max_size, const char *str) {
	size_t str_len = strlen(str);
	
	if (offset + str_len + 1 > max_size) {
		return -1;
	}
	
	memcpy(buffer + offset, str, str_len);
	buffer[offset + str_len] = 0;
	
	return str_len + 1;
}

int udp_create_auth_message(uint8_t *buffer, size_t buffer_size, const char *username, const char *display_name, const char *secret, uint16_t msg_id) {
	if (buffer_size < 3) {
		return -1;
	}
	
	// Header
	buffer[0] = MSG_TYPE_AUTH;
	buffer[1] = msg_id >> 8;   // High byte of message ID
	buffer[2] = msg_id & 0xFF; // Low byte of message ID
	
	size_t offset = 3;
	
	// Username
	int result = write_stringz(buffer, offset, buffer_size, username);
	if (result < 0) return -1;
	offset += result;
	
	// Display Name
	result = write_stringz(buffer, offset, buffer_size, display_name);
	if (result < 0) return -1;
	offset += result;
	
	// Secret
	result = write_stringz(buffer, offset, buffer_size, secret);
	if (result < 0) return -1;
	offset += result;
	
	return offset;
}

int udp_create_join_message(uint8_t *buffer, size_t buffer_size, const char *channel_id, const char *display_name, uint16_t msg_id) {
	if (buffer_size < 3) {
		return -1;
	}
	
	// Header
	buffer[0] = MSG_TYPE_JOIN;
	buffer[1] = msg_id >> 8;   // High byte of message ID
	buffer[2] = msg_id & 0xFF; // Low byte of message ID
	
	size_t offset = 3;
	
	// Channel ID
	int result = write_stringz(buffer, offset, buffer_size, channel_id);
	if (result < 0) return -1;
	offset += result;
	
	// Display Name
	result = write_stringz(buffer, offset, buffer_size, display_name);
	if (result < 0) return -1;
	offset += result;
	
	return offset;
}

int udp_create_msg_message(uint8_t *buffer, size_t buffer_size, const char *display_name, const char *content, uint16_t msg_id) {
	if (buffer_size < 3) {
		return -1;
	}
	
	// Header
	buffer[0] = MSG_TYPE_MSG;
	buffer[1] = msg_id >> 8;   // High byte of message ID
	buffer[2] = msg_id & 0xFF; // Low byte of message ID
	
	size_t offset = 3;
	
	// Display Name
	int result = write_stringz(buffer, offset, buffer_size, display_name);
	if (result < 0) return -1;
	offset += result;
	
	// Content
	result = write_stringz(buffer, offset, buffer_size, content);
	if (result < 0) return -1;
	offset += result;
	
	return offset;
}

int udp_create_bye_message(uint8_t *buffer, size_t buffer_size, const char *display_name, uint16_t msg_id) {
	if (buffer_size < 3) {
		return -1;
	}
	
	// Header
	buffer[0] = MSG_TYPE_BYE;
	buffer[1] = msg_id >> 8;   // High byte of message ID
	buffer[2] = msg_id & 0xFF; // Low byte of message ID
	
	size_t offset = 3;
	
	// Display Name
	int result = write_stringz(buffer, offset, buffer_size, display_name);
	if (result < 0) return -1;
	offset += result;
	
	return offset;
}

int udp_create_confirm_message(uint8_t *buffer, size_t buffer_size, uint16_t ref_msg_id) {
	if (buffer_size < 3) {
		return -1;
	}
	
	// Header
	buffer[0] = MSG_TYPE_CONFIRM;
	buffer[1] = ref_msg_id >> 8;   // High byte of reference message ID
	buffer[2] = ref_msg_id & 0xFF; // Low byte of reference message ID
	
	return 3;
}

/* UDP Message Parsing Function */

// Helper for reading zero-terminated string from buffer
static int read_stringz(const uint8_t *buffer, size_t offset, size_t max_size, char *dest, size_t dest_size) {
	size_t i = 0;
	
	while (offset + i < max_size && i < dest_size - 1) {
		dest[i] = buffer[offset + i];
		if (dest[i] == 0) {
			return i + 1;
		}
		i++;
	}
	
	// Buffer ended or dest buffer is full
	dest[dest_size - 1] = 0;
	
	// Find the end of the string in the buffer
	while (offset + i < max_size) {
		if (buffer[offset + i] == 0) {
			return i + 1;  // Return length including null terminator
		}
		i++;
	}
	
	return -1;  // No null terminator found
}

ResponseType udp_parse_message(const uint8_t *message, size_t message_len, MessageResult *result, uint16_t *msg_id, uint16_t *ref_msg_id) {
	// Reset result structure
	memset(result, 0, sizeof(MessageResult));
	
	if (message_len < 3) {
		printf_debug("Message too short to parse (%zu bytes)", message_len);
		return RESPONSE_NONE;  // Too short
	}
	
	uint8_t msg_type = message[0];
	*msg_id = (message[1] << 8) | message[2];
	
	printf_debug("Parsing UDP message type %d with ID %d", msg_type, *msg_id);
	
	size_t offset = 3;
	
	switch (msg_type) {
		case MSG_TYPE_CONFIRM:
			*ref_msg_id = *msg_id;
			printf_debug("Parsed CONFIRM message for message ID %d", *ref_msg_id);
			return RESPONSE_CONFIRM;
			
		case MSG_TYPE_REPLY: {
			if (offset >= message_len) {
				printf_debug("REPLY message too short");
				return RESPONSE_NONE;
			}
			
			uint8_t result_code = message[offset++];
			
			if (offset + 2 > message_len) {
				printf_debug("REPLY message missing reference ID");
				return RESPONSE_NONE;
			}
			*ref_msg_id = (message[offset] << 8) | message[offset + 1];
			offset += 2;
			
			int content_len = read_stringz(message, offset, message_len, result->content, MAX_MESSAGE_CONTENT_LENGTH + 1);
			if (content_len < 0) {
				printf_debug("REPLY message content missing null terminator");
				return RESPONSE_NONE;
			}
			
			printf_debug("Parsed REPLY message (result: %d, refID: %d, content: %s)", result_code, *ref_msg_id, result->content);
			return (result_code == 1) ? RESPONSE_SUCCESS : RESPONSE_FAILURE;
		}
			
		case MSG_TYPE_MSG: {
			int name_len = read_stringz(message, offset, message_len, result->display_name, MAX_DISPLAYNAME_LENGTH + 1);
			if (name_len < 0) {
				printf_debug("MSG message display name missing null terminator");
				return RESPONSE_NONE;
			}
			offset += name_len;
			
			int content_len = read_stringz(message, offset, message_len, result->content, MAX_MESSAGE_CONTENT_LENGTH + 1);
			if (content_len < 0) {
				printf_debug("MSG message content missing null terminator");
				return RESPONSE_NONE;
			}
			
			printf_debug("Parsed MSG message (from: %s, content: %s)", result->display_name, result->content);
			return RESPONSE_MSG;
		}
			
		case MSG_TYPE_ERR: {
			int name_len = read_stringz(message, offset, message_len, result->display_name, MAX_DISPLAYNAME_LENGTH + 1);
			if (name_len < 0) {
				printf_debug("ERR message display name missing null terminator");
				return RESPONSE_NONE;
			}
			offset += name_len;
			
			int content_len = read_stringz(message, offset, message_len, result->content, MAX_MESSAGE_CONTENT_LENGTH + 1);
			if (content_len < 0) {
				printf_debug("ERR message content missing null terminator");
				return RESPONSE_NONE;
			}
			
			printf_debug("Parsed ERR message (from: %s, content: %s)", result->display_name, result->content);
			return RESPONSE_ERROR;
		}
			
		case MSG_TYPE_BYE: {
			int name_len = read_stringz(message, offset, message_len, result->display_name, MAX_DISPLAYNAME_LENGTH + 1);
			if (name_len < 0) {
				printf_debug("BYE message display name missing null terminator");
				return RESPONSE_NONE;
			}
			
			printf_debug("Parsed BYE message (from: %s)", result->display_name);
			return RESPONSE_BYE;
		}
			
		case MSG_TYPE_PING:
			printf_debug("Parsed PING message");
			return RESPONSE_PING;
			
		default:
			printf_debug("Unknown message type: %d", msg_type);
			return RESPONSE_NONE;
	}
}

/* Common Output Functions */

void print_message_result(const MessageResult *result) {
	switch (result->type) {
		case RESPONSE_SUCCESS:
			printf("Action Success: %s\n", result->content);
			break;
			
		case RESPONSE_FAILURE:
			printf("Action Failure: %s\n", result->content);
			break;
			
		case RESPONSE_ERROR:
			printf("ERROR FROM %s: %s\n", result->display_name, result->content);
			break;
			
		case RESPONSE_MSG:
			printf("%s: %s\n", result->display_name, result->content);
			break;
			
		default:
			break;
	}
}