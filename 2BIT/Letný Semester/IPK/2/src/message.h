/**
 * xbohach00
 * message.h - Message handling functions
 */
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "common.h"

// Response types from the server
typedef enum {
	RESPONSE_NONE,
	RESPONSE_SUCCESS,
	RESPONSE_FAILURE,
	RESPONSE_ERROR,
	RESPONSE_BYE,
	RESPONSE_MSG,
	RESPONSE_CONFIRM,
	RESPONSE_PING
} ResponseType;

// Message structure for UDP
typedef struct {
	uint8_t type;
	uint16_t msg_id;
} UdpMessageHeader;

// Result structure for received messages
typedef struct {
	ResponseType type;
	char display_name[MAX_DISPLAYNAME_LENGTH + 1];
	char content[MAX_MESSAGE_CONTENT_LENGTH + 1];
} MessageResult;

// TCP message functions
int tcp_create_auth_message(char *buffer, size_t buffer_size, const char *username, const char *display_name, const char *secret);
int tcp_create_join_message(char *buffer, size_t buffer_size, const char *channel_id, const char *display_name);
int tcp_create_msg_message(char *buffer, size_t buffer_size, const char *display_name, const char *content);
int tcp_create_bye_message(char *buffer, size_t buffer_size, const char *display_name);
ResponseType tcp_parse_message(const char *message, MessageResult *result);

// UDP message functions
int udp_create_auth_message(uint8_t *buffer, size_t buffer_size, const char *username, const char *display_name, const char *secret, uint16_t msg_id);
int udp_create_join_message(uint8_t *buffer, size_t buffer_size, const char *channel_id, const char *display_name, uint16_t msg_id);
int udp_create_msg_message(uint8_t *buffer, size_t buffer_size, const char *display_name, const char *content, uint16_t msg_id);
int udp_create_bye_message(uint8_t *buffer, size_t buffer_size, const char *display_name, uint16_t msg_id);
int udp_create_confirm_message(uint8_t *buffer, size_t buffer_size, uint16_t ref_msg_id);
ResponseType udp_parse_message(const uint8_t *message, size_t message_len, MessageResult *result, uint16_t *msg_id, uint16_t *ref_msg_id);

// Common output functions
void print_message_result(const MessageResult *result);

#endif /* _MESSAGE_H_ */