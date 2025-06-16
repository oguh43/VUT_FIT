/**
 * xbohach00
 * input.h - Input handling functions
 */
#ifndef _INPUT_H_
#define _INPUT_H_

#include "common.h"

// Command type enum
typedef enum {
	CMD_NONE,
	CMD_AUTH,
	CMD_JOIN,
	CMD_RENAME,
	CMD_HELP,
	CMD_MESSAGE
} CommandType;

// Command structure
typedef struct {
	CommandType type;
	char username[MAX_USERNAME_LENGTH + 1];
	char secret[MAX_SECRET_LENGTH + 1];
	char display_name[MAX_DISPLAYNAME_LENGTH + 1];
	char channel_id[MAX_CHANNELID_LENGTH + 1];
	char message[MAX_MESSAGE_CONTENT_LENGTH + 1];
} Command;

// Initialize input handling
void init_input_handler(void);

// Process input line and populate command structure
bool process_input_line(const char *line, Command *cmd);

// Check if a command is valid in the current state according to the FSM
bool is_command_valid(const Command *cmd, ClientState state);

// Display help message
void display_help(void);

#endif /* _INPUT_H_ */