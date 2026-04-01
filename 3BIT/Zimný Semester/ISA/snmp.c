/***************************************************************************************
*                                                                                      *
*                            Brno University of Technology                             *
*                          Faculty of Information Technology                           *
*                                                                                      *
*                   Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)                 *
*                                                                                      *
*                Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]                *
*                                   Brno 2025                                          *
*                                                                                      *
* Implementation of the SNMP protocol implementation with ASN.1 BER encoding/decoding  *
*                                                                                      *
***************************************************************************************/

#include "snmp2otel.h"

/* ASN.1 BER encoding functions */
int encode_length(unsigned char *buffer, int length) {
	if (length < 0x80) {
		/* Short form */
		buffer[0] = (unsigned char)length;
		return 1;
	} else {
		/* Long form */
		int byte_count = 0;
		int temp_length = length;
		
		/* Count bytes needed */
		while (temp_length > 0) {
			byte_count++;
			temp_length >>= 8;
		}
		
		buffer[0] = (unsigned char) (0x80 | byte_count);
		
		/* Encode length bytes in big-endian order */
		for (int i = byte_count; i > 0; i--) {
			buffer[i] = (unsigned char) (length & 0xFF);
			length >>= 8;
		}
		
		return byte_count + 1;
	}
}

int decode_length(const unsigned char *buffer, int *length, int *bytes_read) {
	if (buffer[0] & 0x80) {
		/* Long form */
		int byte_count = buffer[0] & 0x7F;
		if (byte_count == 0 || byte_count > 4) {
			return -1; /* Invalid or unsupported */
		}
		
		*length = 0;
		for (int i = 1; i <= byte_count; i++) {
			*length = (*length << 8) | buffer[i];
		}
		*bytes_read = byte_count + 1;
	} else {
		/* Short form */
		*length = buffer[0];
		*bytes_read = 1;
	}
	
	return 0;
}

int encode_integer(unsigned char *buffer, int value) {
	unsigned char *start = buffer;
	
	/* Tag */
	*buffer++ = ASN1_INTEGER;
	
	/* Determine number of bytes needed */
	int bytes_needed;
	if (value == 0) {
		bytes_needed = 1;
	} else {
		bytes_needed = 0;
		int temp = value;
		if (value < 0) {
			temp = -value - 1;
		}
		while (temp > 0) {
			bytes_needed++;
			temp >>= 8;
		}
		
		/* Sign bit? */
		if (value >= 0 && (value >> ((bytes_needed - 1) * 8)) & 0x80) {
			bytes_needed++;
		}
	}
	
	/* Length */
	buffer += encode_length(buffer, bytes_needed);
	
	/* Value */
	if (value == 0) {
		*buffer++ = 0;
	} else {
		for (int i = bytes_needed - 1; i >= 0; i--) {
			*buffer++ = (unsigned char) ((value >> (i * 8)) & 0xFF);
		}
	}
	
	return (int) (buffer - start);
}

int decode_integer(const unsigned char *buffer, int length, int *value) {
	if (length <= 0 || length > 4) {
		return -1;
	}
	
	*value = 0;
	
	/* Check for sign bit */
	int is_negative = (buffer[0] & 0x80) != 0;
	
	for (int i = 0; i < length; i++) {
		*value = (*value << 8) | buffer[i];
	}
	
	/* Handle negative values (two's complement) */
	if (is_negative && length < 4) {
		/* Sign extend */
		*value |= (int) ((~0U) << (length * 8));
	}
	
	return 0;
}

int encode_string(unsigned char *buffer, const char *str, int len) {
	unsigned char *start = buffer;
	
	/* Tag */
	*buffer++ = ASN1_OCTET_STRING;
	
	/* Length */
	buffer += encode_length(buffer, len);
	
	/* Value */
	memcpy(buffer, str, (size_t) len);
	buffer += len;
	
	return (int) (buffer - start);
}

int decode_string(const unsigned char *buffer, int length, char *str, int max_len) {
	if (length < 0 || length >= max_len) {
		return -1;
	}
	
	memcpy(str, buffer, (size_t) length);
	str[length] = '\0';
	
	return 0;
}

int encode_oid(unsigned char *buffer, const oid_t *oid) {
	const unsigned char *start = buffer;
	unsigned char temp_buffer[MAX_OID_LEN * 5];
	int temp_length = 0;
	
	if (buffer == NULL || oid == NULL) {
		return -1;
	}

	if (oid->length < 2) {
		return -1;
	}
	
	/* First two numbers are encoded as: (first * 40) + second */
	unsigned int first_encoded = oid->numbers[0] * 40 + oid->numbers[1];
	
	/* Encode first combined value */
	if (first_encoded < 0x80) {
		temp_buffer[temp_length++] = (unsigned char) first_encoded;
	} else {
		/* Multi-byte encoding */
		unsigned char bytes[5];
		int byte_count = 0;
		unsigned int temp = first_encoded;
		
		while (temp > 0) {
			bytes[byte_count] = (temp & 0x7F) | (byte_count > 0 ? 0x80 : 0);
			byte_count++;
			temp >>= 7;
		}
		
		/* Reverse byte order */
		for (int i = byte_count - 1; i >= 0; i--) {
			temp_buffer[temp_length++] = bytes[i];
		}
	}
	
	/* Encode remaining OID components */
	for (int i = 2; i < oid->length; i++) {
		unsigned int component = oid->numbers[i];
		
		if (component < 0x80) {
			temp_buffer[temp_length++] = (unsigned char) component;
		} else {
			/* Multi-byte encoding */
			unsigned char bytes[5];
			int byte_count = 0;
			unsigned int temp = component;
			
			/* Encode in base-128 with continuation bits */
			bytes[byte_count++] = temp & 0x7F;
			temp >>= 7;
			
			while (temp > 0) {
				bytes[byte_count++] = (temp & 0x7F) | 0x80;
				temp >>= 7;
			}
			
			/* Write bytes in reverse order */
			for (int j = byte_count - 1; j >= 0; j--) {
				temp_buffer[temp_length++] = bytes[j];
			}
		}
	}
	
	/* Tag */
	*buffer++ = ASN1_OBJECT_ID;
	
	/* Length */
	buffer += encode_length(buffer, temp_length);
	
	/* Value */
	memcpy(buffer, temp_buffer, (size_t) temp_length);
	buffer += temp_length;
	
	return (int) (buffer - start);
}

int decode_oid(const unsigned char *buffer, int length, oid_t *oid) {
	int pos = 0;
	int oid_index = 0;
	
	if (length <= 0) {
		return -1;
	}
	
	/* Decode first combined value */
	unsigned int first_combined = buffer[pos++];
	oid->numbers[oid_index++] = first_combined / 40;
	oid->numbers[oid_index++] = first_combined % 40;
	
	/* Decode remaining components */
	while (pos < length && oid_index < MAX_OID_LEN) {
		unsigned int component = 0;
		
		do {
			if (pos >= length) {
				return -1;
			}
			component = (component << 7) | (buffer[pos] & 0x7F);
		} while (buffer[pos++] & 0x80);
		
		oid->numbers[oid_index++] = component;
	}
	
	oid->length = oid_index;
	
	/* Create string representation */
	snprintf(oid->str, sizeof(oid->str), "%u", oid->numbers[0]);
	for (int i = 1; i < oid->length; i++) {
		char temp[32];
		snprintf(temp, sizeof(temp), ".%u", oid->numbers[i]);
		strncat(oid->str, temp, sizeof(oid->str) - strlen(oid->str) - 1);
	}
	
	return 0;
}

int build_snmp_get_request(unsigned char *buffer, const char *community, const oid_list_t *oids, int request_id) {
	unsigned char *start = buffer;
	unsigned char *seq_start, *pdu_start, *varbind_list_start;
	int community_len = (int) strlen(community);
	
	/* SNMP Message Sequence */
	*buffer++ = ASN1_SEQUENCE;
	seq_start = buffer++;
	
	/* Version (INTEGER 1 for SNMPv2c) */
	buffer += encode_integer(buffer, SNMP_VERSION_2C);
	
	/* Community (OCTET STRING) */
	buffer += encode_string(buffer, community, community_len);
	
	/* PDU (GetRequest) */
	*buffer++ = 0xA0; /* Context-specific, constructed, tag 0 */
	pdu_start = buffer++;
	
	/* Request ID */
	buffer += encode_integer(buffer, request_id);
	
	/* Error Status */
	buffer += encode_integer(buffer, 0);
	
	/* Error Index */
	buffer += encode_integer(buffer, 0);
	
	/* Variable Bindings (SEQUENCE) */
	*buffer++ = ASN1_SEQUENCE;
	varbind_list_start = buffer++;
	
	/* Add each OID with NULL value */
	for (int i = 0; i < oids->count; i++) {
		unsigned char *varbind_start = buffer;
		
		/* Variable Binding (SEQUENCE) */
		*buffer++ = ASN1_SEQUENCE;
		unsigned char *varbind_len_pos = buffer++;
		
		/* OID */
		buffer += encode_oid(buffer, &oids->oids[i]);
		
		/* Value (NULL) */
		*buffer++ = ASN1_NULL;
		*buffer++ = 0;
		
		/* Update varbind length */
		int varbind_length = (int) (buffer - varbind_start - 2);
		*varbind_len_pos = (unsigned char) varbind_length;
	}
	
	/* Update lengths */
	int varbind_list_length = (int) (buffer - varbind_list_start - 1);
	*varbind_list_start = (unsigned char) varbind_list_length;
	
	int pdu_length = (int) (buffer - pdu_start - 1);
	*pdu_start = (unsigned char) pdu_length;
	
	int total_length = (int) (buffer - seq_start - 1);
	*seq_start = (unsigned char) total_length;
	
	return (int) (buffer - start);
}

int parse_snmp_response(const unsigned char *buffer, int length, snmp_var_t *vars, int max_vars, int *var_count) {
	int pos = 0;
	int len_bytes, content_length;
	
	*var_count = 0;
	
	/* Parse outer sequence */
	if (pos >= length || buffer[pos] != ASN1_SEQUENCE) {
		log_error("Invalid SNMP response: expected SEQUENCE\n");
		return -1;
	}
	pos++;
	
	if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
		log_error("Invalid SNMP response: bad length encoding\n");
		return -1;
	}
	pos += len_bytes;
	
	/* Skip version */
	if (pos >= length || buffer[pos] != ASN1_INTEGER) {
		log_error("Invalid SNMP response: expected version\n");
		return -1;
	}
	pos++;
	
	if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
		return -1;
	}
	pos += len_bytes + content_length;
	
	/* Skip community */
	if (pos >= length || buffer[pos] != ASN1_OCTET_STRING) {
		log_error("Invalid SNMP response: expected community\n");
		return -1;
	}
	pos++;
	
	if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
		return -1;
	}
	pos += len_bytes + content_length;
	
	/* Parse PDU (should be GetResponse = 0xA2) */
	if (pos >= length || buffer[pos] != 0xA2) {
		log_error("Invalid SNMP response: expected GetResponse PDU\n");
		return -1;
	}
	pos++;
	
	if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
		return -1;
	}
	pos += len_bytes;
	
	/* Skip request ID, error status, error index */
	for (int i = 0; i < 3; i++) {
		if (pos >= length || buffer[pos] != ASN1_INTEGER) {
			log_error("Invalid SNMP response: expected INTEGER\n");
			return -1;
		}
		pos++;
		
		if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
			return -1;
		}
		pos += len_bytes + content_length;
	}
	
	/* Parse variable bindings */
	if (pos >= length || buffer[pos] != ASN1_SEQUENCE) {
		log_error("Invalid SNMP response: expected varbind list\n");
		return -1;
	}
	pos++;
	
	if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
		return -1;
	}
	pos += len_bytes;
	
	int varbind_end = pos + content_length;
	
	/* Parse each variable binding */
	while (pos < varbind_end && *var_count < max_vars) {
		if (buffer[pos] != ASN1_SEQUENCE) {
			log_error("Invalid varbind: expected SEQUENCE\n");
			return -1;
		}
		pos++;
		
		if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
			return -1;
		}
		pos += len_bytes;
		
		/* Parse OID */
		if (buffer[pos] != ASN1_OBJECT_ID) {
			log_error("Invalid varbind: expected OID\n");
			return -1;
		}
		pos++;
		
		if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
			return -1;
		}
		pos += len_bytes;
		
		if (decode_oid(&buffer[pos], content_length, &vars[*var_count].oid) != 0) {
			log_error("Failed to decode OID\n");
			return -1;
		}
		pos += content_length;
		
		/* Parse value */
		unsigned char value_tag = buffer[pos];
		pos++;
		
		if (decode_length(&buffer[pos], &content_length, &len_bytes) != 0) {
			return -1;
		}
		pos += len_bytes;
		
		vars[*var_count].type = value_tag;
		
		switch (value_tag) {
			case ASN1_INTEGER:
				if (decode_integer(&buffer[pos], content_length, &vars[*var_count].value.integer_val) != 0) {
					log_error("Failed to decode INTEGER value\n");
					return -1;
				}
				break;
				
			case ASN1_OCTET_STRING:
				if (decode_string(&buffer[pos], content_length, vars[*var_count].value.string_val, sizeof(vars[*var_count].value.string_val)) != 0) {
					log_error("Failed to decode OCTET STRING value\n");
					return -1;
				}
				break;
				
			case ASN1_OBJECT_ID:
				if (decode_oid(&buffer[pos], content_length, &vars[*var_count].value.oid_val) != 0) {
					log_error("Failed to decode OID value\n");
					return -1;
				}
				break;
				
			default:
				log_message(verbose_mode, "Unsupported value type: 0x%02X\n", value_tag);
				pos += content_length;
				continue;
		}
		
		pos += content_length;
		(*var_count)++;
	}
	
	return 0;
}

int send_snmp_request(const config_t *config, const oid_list_t *oids, snmp_var_t *results, int *result_count) {
	int sockfd;
	struct sockaddr_in server_addr;
	unsigned char request_buffer[MAX_BUFFER_SIZE];
	unsigned char response_buffer[MAX_BUFFER_SIZE];
	int request_len;
	int request_id = (int)time(NULL);
	struct timeval timeout;
	socklen_t addr_len;
	
	*result_count = 0;
	
	/* Create UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		log_error("Failed to create socket: %s\n", strerror(errno));
		return -1;
	}
	
	/* Set socket timeout */
	timeout.tv_sec = config->timeout / 1000;
	timeout.tv_usec = (config->timeout % 1000) * 1000;
	
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		log_error("Failed to set socket timeout: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	
	/* Resolve hostname and set up server address */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t) config->port);
	
	if (inet_pton(AF_INET, config->target, &server_addr.sin_addr) <= 0) {
		struct addrinfo hints, *result;
		
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		
		if (getaddrinfo(config->target, NULL, &hints, &result) != 0) {
			log_error("Failed to resolve hostname: %s\n", config->target);
			close(sockfd);
			return -1;
		}
		
		memcpy(&server_addr, result->ai_addr, sizeof(server_addr));
		server_addr.sin_port = htons((uint16_t) config->port);
		freeaddrinfo(result);
	}
	
	/* Build SNMP request */
	request_len = build_snmp_get_request(request_buffer, config->community, oids, request_id);
	if (request_len <= 0) {
		log_error("Failed to build SNMP request\n");
		close(sockfd);
		return -1;
	}
	
	log_message(config->verbose, "Sending SNMP request (%d bytes) to %s:%d\n", request_len, config->target, config->port);
	
	/* Send request with retries */
	for (int retry = 0; retry <= config->retries; retry++) {
		if (sendto(sockfd, request_buffer, (size_t) request_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			log_error("Failed to send SNMP request: %s\n", strerror(errno));
			close(sockfd);
			return -1;
		}
		
		/* Wait for response */
		addr_len = sizeof(server_addr);
		int response_len = (int) recvfrom(sockfd, response_buffer, sizeof(response_buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
		
		if (response_len > 0) {
			log_message(config->verbose, "Received SNMP response (%d bytes)\n", response_len);
			
			/* Parse response */
			if (parse_snmp_response(response_buffer, response_len, results, MAX_OID_COUNT, result_count) == 0) {
				close(sockfd);
				return 0;
			} else {
				log_error("Failed to parse SNMP response\n");
				break;
			}
		} else if (response_len < 0 && errno == EAGAIN) {
			log_message(config->verbose, "SNMP request timeout (attempt %d/%d)\n", retry + 1, config->retries + 1);
		} else {
			log_error("Failed to receive SNMP response: %s\n", strerror(errno));
			break;
		}
	}
	
	close(sockfd);
	return -1;
}