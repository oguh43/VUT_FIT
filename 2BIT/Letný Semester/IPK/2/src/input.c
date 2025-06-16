/**
 * xbohach00
 * input.c - Input handling implementation
 */
#include "input.h"
#include <ctype.h>

// Initialize input handling
void init_input_handler(void) {
	setvbuf(stdin, NULL, _IONBF, 0);
	printf_debug("Input handler initialized");
}

// Helper function to check if username/channelid is valid
static bool is_valid_id(const char *id, size_t max_length) {
	if (!id || strlen(id) > max_length) {
		return false;
	}
	
	for (size_t i = 0; i < strlen(id); i++) {
		char c = id[i];
		if (!(isalnum(c) || c == '_' || c == '-')) {
			return false;
		}
	}
	
	return true;
}

// Helper function to check if display name is valid
static bool is_valid_display_name(const char *name) {
	if (!name || strlen(name) > MAX_DISPLAYNAME_LENGTH) {
		return false;
	}
	
	for (size_t i = 0; i < strlen(name); i++) {
		unsigned char c = (unsigned char)name[i];
		if (c < 0x21 || c > 0x7E) {
			return false;
		}
	}
	
	return true;
}

// Process input line and populate command structure
bool process_input_line(const char *line, Command *cmd) {
	if (!line || !cmd) {
		return false;
	}
	
	// Reset command structure
	memset(cmd, 0, sizeof(Command));
	
	// Skip leading whitespace
	while (isspace(*line)) {
		line++;
	}
	
	// Check if this is a command (starts with '/')
	if (line[0] == '/') {
		line++;
		
		char command[32] = {0};
		int args_start = 0;
		
		// Read command name
		if (sscanf(line, "%31s%n", command, &args_start) != 1) {
			return false;
		}
		
		// Move to command arguments
		line += args_start;
		
		printf_debug("Processing command: /%s", command);
		
		// Process specific commands
		if (strcmp(command, "auth") == 0) {
			cmd->type = CMD_AUTH;
			
			char username[MAX_USERNAME_LENGTH + 1] = {0};
			char secret[MAX_SECRET_LENGTH + 1] = {0};
			char display_name[MAX_DISPLAYNAME_LENGTH + 1] = {0};
			
			while (isspace(*line)) line++;
			
			// Read username
			int i = 0;
			while (*line && !isspace(*line) && i < MAX_USERNAME_LENGTH) {
				username[i++] = *line++;
			}
			username[i] = '\0';
			
			while (isspace(*line)) line++;
			
			// Read secret
			i = 0;
			while (*line && !isspace(*line) && i < MAX_SECRET_LENGTH) {
				secret[i++] = *line++;
			}
			secret[i] = '\0';
			
			while (isspace(*line)) line++;
			
			// Read display name (rest of the line)
			i = 0;
			while (*line && i < MAX_DISPLAYNAME_LENGTH) {
				display_name[i++] = *line++;
			}
			display_name[i] = '\0';
			
			// Validate input
			if (!is_valid_id(username, MAX_USERNAME_LENGTH)) {
				print_error("Invalid username format");
				return false;
			}
			
			if (!is_valid_id(secret, MAX_SECRET_LENGTH)) {
				print_error("Invalid secret format");
				return false;
			}
			
			if (!is_valid_display_name(display_name)) {
				print_error("Invalid display name format");
				return false;
			}
			
			strncpy(cmd->username, username, MAX_USERNAME_LENGTH);
			strncpy(cmd->secret, secret, MAX_SECRET_LENGTH);
			strncpy(cmd->display_name, display_name, MAX_DISPLAYNAME_LENGTH);
			
			printf_debug("Processed AUTH command (username: %s, display_name: %s)", username, display_name);
			
		} else if (strcmp(command, "join") == 0) {
			cmd->type = CMD_JOIN;
			
			char channel_id[MAX_CHANNELID_LENGTH + 1] = {0};
			
			while (isspace(*line)) line++;
			
			// Read channel ID
			int i = 0;
			while (*line && !isspace(*line) && i < MAX_CHANNELID_LENGTH) {
				channel_id[i++] = *line++;
			}
			channel_id[i] = '\0';
			
			// Validate input
			if (!is_valid_id(channel_id, MAX_CHANNELID_LENGTH)) {
				print_error("Invalid channel ID format");
				return false;
			}
			
			strncpy(cmd->channel_id, channel_id, MAX_CHANNELID_LENGTH);
			
			printf_debug("Processed JOIN command (channel_id: %s)", channel_id);
			
		} else if (strcmp(command, "rename") == 0) {
			cmd->type = CMD_RENAME;
			
			char display_name[MAX_DISPLAYNAME_LENGTH + 1] = {0};
			
			while (isspace(*line)) line++;
			
			// Read display name (rest of the line)
			int i = 0;
			while (*line && i < MAX_DISPLAYNAME_LENGTH) {
				display_name[i++] = *line++;
			}
			display_name[i] = '\0';
			
			// Validate input
			if (!is_valid_display_name(display_name)) {
				print_error("Invalid display name format");
				return false;
			}
			
			strncpy(cmd->display_name, display_name, MAX_DISPLAYNAME_LENGTH);
			
			printf_debug("Processed RENAME command (display_name: %s)", display_name);
			
		} else if (strcmp(command, "help") == 0) {
			cmd->type = CMD_HELP;
			printf_debug("Processed HELP command");
			
		} else {
			print_error("Unknown command: /%s", command);
			return false;
		}
		
	} else {
		// It's a regular message
		cmd->type = CMD_MESSAGE;
		
		// Copy the message content
		strncpy(cmd->message, line, MAX_MESSAGE_CONTENT_LENGTH);
		
		// Validate message content
		for (size_t i = 0; i < strlen(cmd->message); i++) {
			unsigned char c = (unsigned char)cmd->message[i];
			if (!((c >= 0x20 && c <= 0x7E) || c == 0x0A)) {
				print_error("Message contains invalid characters");
				return false;
			}
		}
		
		printf_debug("Processed MESSAGE (length: %zu)", strlen(cmd->message));
	}
	
	return true;
}

// Check if a command is valid in the current state
bool is_command_valid(const Command *cmd, ClientState state) {
	if (!cmd) {
		return false;
	}
	
	switch (cmd->type) {
		case CMD_AUTH:
			return (state == STATE_START);
			
		case CMD_JOIN:
			return (state == STATE_OPEN);
			
		case CMD_RENAME:
			return true;  // Can be used in any state
			
		case CMD_HELP:
			return true;  // Can be used in any state
			
		case CMD_MESSAGE:
			return (state == STATE_OPEN);
			
		default:
			return false;
	}
}

// Display help message
void display_help(void) {
	printf("Supported commands:\n");
	printf("  /auth {Username} {Secret} {DisplayName} - Authenticate with the server\n");
	printf("  /join {ChannelID} - Join a channel\n");
	printf("  /rename {DisplayName} - Change your display name\n");
	printf("  /help - Display this help message\n");
	printf("\nAny input not starting with '/' is treated as a message to the current channel.\n");
}