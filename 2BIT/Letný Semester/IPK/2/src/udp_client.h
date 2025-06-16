/**
 * xbohach00
 * udp_client.h - UDP protocol variant implementation
 */
#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include "common.h"
#include "input.h"
#include "message.h"

// Initialize UDP client
bool udp_client_init(const char *host, int port, int timeout_ms, int max_retries);

// Process user command in UDP mode
bool udp_client_process_command(const Command *cmd);

// UDP client main loop
void udp_client_run(void);

// Clean up UDP client resources
void udp_client_cleanup(void);

#endif /* _UDP_CLIENT_H_ */