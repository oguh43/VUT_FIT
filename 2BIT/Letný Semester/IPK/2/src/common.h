/**
 * xbohach00
 * common.h - Common definitions and structures for the application
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <ctype.h>
#include <time.h>

// Debug logging macro - fixed for C99 compliance
#ifdef DEBUG_PRINT
#define printf_debug(...) \
        do { fprintf(stderr, "%s:%d:%s(): ",__FILE__, __LINE__, __func__);\
             fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define printf_debug(...) ((void)0)
#endif

// Default values
#define DEFAULT_PORT 4567
#define DEFAULT_TIMEOUT_MS 250
#define DEFAULT_RETRIES 3

// Message type constants for UDP variant
#define MSG_TYPE_CONFIRM 0x00
#define MSG_TYPE_REPLY 0x01
#define MSG_TYPE_AUTH 0x02
#define MSG_TYPE_JOIN 0x03
#define MSG_TYPE_MSG 0x04
#define MSG_TYPE_PING 0xFD
#define MSG_TYPE_ERR 0xFE
#define MSG_TYPE_BYE 0xFF

// Maximum lengths for different message parameters
#define MAX_USERNAME_LENGTH 20
#define MAX_CHANNELID_LENGTH 20
#define MAX_SECRET_LENGTH 128
#define MAX_DISPLAYNAME_LENGTH 20
#define MAX_MESSAGE_CONTENT_LENGTH 60000
#define MAX_BUFFER_SIZE 65535

// Client states FSM
typedef enum {
	STATE_START,
	STATE_AUTH,
	STATE_OPEN,
	STATE_END
} ClientState;

// Protocol type
typedef enum {
	PROTO_TCP,
	PROTO_UDP
} ProtocolType;

// Client config
typedef struct {
	ProtocolType protocol;
	char *server_host;
	int server_port;
	int timeout_ms;     // UDP confirmation timeout
	int max_retries;    // UDP max retransmissions
	char display_name[MAX_DISPLAYNAME_LENGTH + 1];
	ClientState state;  // FSM state
	bool running;
	int socket_fd;
	int server_dyn_port; // UDP dynamic server port
	struct sockaddr_in server_dyn_addr;  // UDP dynamic server address
	uint16_t next_message_id;
} ClientConfig;

// Error handling
void print_error(const char *format, ...);
void exit_error(const char *format, ...);

// Shared global variables
extern ClientConfig config;
extern volatile sig_atomic_t termination_requested;

#endif /* _COMMON_H_ */
