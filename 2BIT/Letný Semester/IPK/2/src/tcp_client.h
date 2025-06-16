/**
 * xbohach00
 * tcp_client.h - TCP protocol variant implementation
 */
#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "common.h"
#include "input.h"
#include "message.h"

// Initialize TCP client
bool tcp_client_init(const char *host, int port);

// Process user command in TCP mode
bool tcp_client_process_command(const Command *cmd);

// Run the TCP client main loop
void tcp_client_run(void);

// Clean up TCP client resources
void tcp_client_cleanup(void);

#endif /* _TCP_CLIENT_H_ */