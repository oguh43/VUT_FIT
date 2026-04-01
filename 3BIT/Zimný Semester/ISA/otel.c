/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*               Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)             *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*     Implementation of the OpenTelemetry OTLP/HTTP JSON export functions      *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"

/* JSON escaping */
void json_escape_string(const char *input, char *output, int max_len) {
	int input_len = (int) strlen(input);
	int output_pos = 0;
	
	if (max_len <= 0){
		return;
	}

	for (int i = 0; i < input_len && output_pos < max_len - 2; i++) {
		switch (input[i]) {
			case '"':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = '"';
				}
				break;
			case '\\':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = '\\';
				}
				break;
			case '\b':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = 'b';
				}
				break;
			case '\f':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = 'f';
				}
				break;
			case '\n':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = 'n';
				}
				break;
			case '\r':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = 'r';
				}
				break;
			case '\t':
				if (output_pos < max_len - 3) {
					output[output_pos++] = '\\';
					output[output_pos++] = 't';
				}
				break;
			default:
				if (input[i] < 32) {
					/* Control character - encode as \uXXXX */
					if (output_pos < max_len - 7) {
						snprintf(&output[output_pos], 7, "\\u%04X", (unsigned char)input[i]);
						output_pos += 6;
					}
				} else {
					if (output_pos < max_len - 1) {
						output[output_pos++] = input[i];
					}
				}
				break;
		}
	}
	output[output_pos] = '\0';
}

/* Generate metric name from OID - Replace dots with underscores for metric name*/
void generate_metric_name_from_oid(const oid_t *oid, char *name, int max_len) {
	snprintf(name, (size_t) max_len, "snmp.oid.%s", oid->str);
	
	for (char *p = name; *p; p++) {
		if (*p == '.') {
			*p = '_';
		}
	}
}

/* Convert SNMP value to string for JSON */
void snmp_value_to_string(const snmp_var_t *var, char *value_str, int max_len) {
	switch (var->type) {
		case ASN1_INTEGER:
			snprintf(value_str, (size_t) max_len, "%d", var->value.integer_val);
			break;
		case ASN1_OCTET_STRING:
			/* For OCTET STRING, we'll try to interpret as a number if possible */
			{
				char *endptr;
				long val = strtol(var->value.string_val, &endptr, 10);
				if (*endptr == '\0') {
					/* Valid number */
					snprintf(value_str, (size_t) max_len, "%ld", val);
				} else {
					/* Treat as string, but use length as value for gauge */
					snprintf(value_str, (size_t) max_len, "%d", (int)strlen(var->value.string_val));
				}
			}
			break;
		case ASN1_OBJECT_ID:
			/* For OID values, use length as the gauge value */
			snprintf(value_str, (size_t) max_len, "%d", var->value.oid_val.length);
			break;
		default:
			/* Unknown type - default to 0 */
			strcpy(value_str, "0");
			break;
	}
}

int build_otel_json(const snmp_var_t *vars, int var_count, char *json_buffer, int buffer_size) {
	int pos = 0;
	unsigned long timestamp_ns = get_timestamp_ms() * 1000000UL; /* Convert to nanoseconds */
	char escaped_string[512];
	char metric_name[256];
	char value_str[64];
	
	/* Start JSON structure */
	pos += snprintf(json_buffer + pos, (size_t) (buffer_size - pos),
		"{\n"
		"  \"resourceMetrics\": [\n"
		"    {\n"
		"      \"resource\": {\n"
		"        \"attributes\": [\n"
		"          {\n"
		"            \"key\": \"service.name\",\n"
		"            \"value\": { \"stringValue\": \"snmp2otel\" }\n"
		"          },\n"
		"          {\n"
		"            \"key\": \"service.version\",\n"
		"            \"value\": { \"stringValue\": \"1.0\" }\n"
		"          }\n"
		"        ]\n"
		"      },\n"
		"      \"scopeMetrics\": [\n"
		"        {\n"
		"          \"scope\": {\n"
		"            \"name\": \"snmp2otel\",\n"
		"            \"version\": \"1.0\"\n"
		"          },\n"
		"          \"metrics\": [\n");
	
	if (pos >= buffer_size - 1) {
		log_error("JSON buffer too small\n");
		return -1;
	}
	
	/* Add each metric */
	for (int i = 0; i < var_count; i++) {
		generate_metric_name_from_oid(&vars[i].oid, metric_name, sizeof(metric_name));
		json_escape_string(metric_name, escaped_string, sizeof(escaped_string));
		snmp_value_to_string(&vars[i], value_str, sizeof(value_str));
		
		pos += snprintf(json_buffer + pos, (size_t) (buffer_size - pos),
			"%s            {\n"
			"              \"name\": \"%s\",\n"
			"              \"description\": \"SNMP metric for OID %s\",\n"
			"              \"unit\": \"\",\n"
			"              \"gauge\": {\n"
			"                \"dataPoints\": [\n"
			"                  {\n"
			"                    \"attributes\": [\n"
			"                      {\n"
			"                        \"key\": \"snmp.oid\",\n"
			"                        \"value\": { \"stringValue\": \"%s\" }\n"
			"                      }\n"
			"                    ],\n"
			"                    \"timeUnixNano\": \"%lu\",\n"
			"                    \"asDouble\": %s\n"
			"                  }\n"
			"                ]\n"
			"              }\n"
			"            }",
			(i > 0) ? ",\n" : "",
			escaped_string,
			vars[i].oid.str,
			vars[i].oid.str,
			timestamp_ns,
			value_str);
		
		if (pos >= buffer_size - 1) {
			log_error("JSON buffer too small for metric %d\n", i);
			return -1;
		}
	}
	
	/* Close JSON structure */
	pos += snprintf(json_buffer + pos, (size_t) (buffer_size - pos),
		"\n          ]\n"
		"        }\n"
		"      ]\n"
		"    }\n"
		"  ]\n"
		"}\n");
	
	if (pos >= buffer_size - 1) {
		log_error("JSON buffer too small for closing\n");
		return -1;
	}
	
	return pos;
}

int export_to_otel(const config_t *config, const snmp_var_t *vars, int var_count) {
	char json_buffer[MAX_JSON_SIZE];
	char host[MAX_HOSTNAME_LEN];
	char path[MAX_URL_LEN];
	int port;
	int use_https;
	int json_len;
	
	if (var_count == 0) {
		log_message(config->verbose, "No metrics to export\n");
		return 0;
	}
	
	/* Parse OTEL endpoint URL */
	if (parse_url(config->endpoint, host, &port, path, &use_https) != 0) {
		log_error("Failed to parse OTEL endpoint URL: %s\n", config->endpoint);
		return -1;
	}
	
	log_message(config->verbose, "OTEL endpoint: %s:%d%s (HTTPS: %s)\n", host, port, path, use_https ? "yes" : "no");
	
	/* Build OTLP JSON payload */
	json_len = build_otel_json(vars, var_count, json_buffer, sizeof(json_buffer));
	if (json_len <= 0) {
		log_error("Failed to build OTLP JSON payload\n");
		return -1;
	}
	
	log_message(config->verbose, "Generated OTLP JSON payload (%d bytes)\n", json_len);
	if (config->verbose) {
		log_message(1, "JSON payload:\n%s\n", json_buffer);
	}
	
	/* Send HTTP request */
	if (send_http_request(host, port, path, json_buffer, use_https) != 0) {
		log_error("Failed to send OTLP data to endpoint\n");
		return -1;
	}
	
	log_message(config->verbose, "Successfully exported %d metrics to OTEL\n", var_count);
	return 0;
}