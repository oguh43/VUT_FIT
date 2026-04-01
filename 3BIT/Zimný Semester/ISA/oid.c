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
*            Implementation of the OID parsing and handling functions          *
*                                                                              *
*******************************************************************************/

#include "snmp2otel.h"

int parse_oid_string(const char *oid_str, oid_t *oid) {
	char buffer[MAX_LINE_LEN];
	char *token;
	int i = 0;
	
	if (!oid_str || !oid) {
		return -1;
	}
	
	/* Initialize OID structure */
	memset(oid, 0, sizeof(oid_t));
	
	strncpy(buffer, oid_str, MAX_LINE_LEN - 1);
	buffer[MAX_LINE_LEN - 1] = '\0';
	
	/* Skip to the interesting parts */
	char *start = buffer;
	while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
		start++;
	}
	
	char *end = start + strlen(start) - 1;
	while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
		*end = '\0';
		end--;
	}
	
	if (*start == '.') {
		start++;
	}
	
	/* Parse OID numbers */
	token = strtok(start, ".");
	while (token != NULL && i < MAX_OID_LEN) {
		char *endptr;
		long num = strtol(token, &endptr, 10);
		
		if (*endptr != '\0' || num < 0 || num > UINT_MAX) {
			log_error("Invalid OID component: %s\n", token);
			return -1;
		}
		
		oid->numbers[i] = (unsigned int)num;
		i++;
		token = strtok(NULL, ".");
	}
	
	if (i == 0) {
		log_error("Empty OID\n");
		return -1;
	}
	
	if (token != NULL) {
		log_error("OID too long (max %d components)\n", MAX_OID_LEN);
		return -1;
	}
	
	oid->length = i;
	
	/* Store string representation */
	snprintf(oid->str, sizeof(oid->str), "%s", oid_str);
	
	/* Normalize string representation */
	snprintf(oid->str, sizeof(oid->str), "%u", oid->numbers[0]);
	for (int j = 1; j < oid->length; j++) {
		char temp[32];
		snprintf(temp, sizeof(temp), ".%u", oid->numbers[j]);
		strncat(oid->str, temp, sizeof(oid->str) - strlen(oid->str) - 1);
	}
	
	return 0;
}

int load_oids_from_file(const char *filename, oid_list_t *oid_list) {
	FILE *file;
	char line[MAX_LINE_LEN];
	int line_number = 0;
	
	if (!filename || !oid_list) {
		return -1;
	}
	
	/* Initialize OID list */
	memset(oid_list, 0, sizeof(oid_list_t));
	
	/* Open file and read from it */
	file = fopen(filename, "r");
	if (!file) {
		log_error("Cannot open OID file '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	
	while (fgets(line, sizeof(line), file) != NULL && oid_list->count < MAX_OID_COUNT) {
		line_number++;
		
		char *newline = strchr(line, '\n');
		if (newline) {
			*newline = '\0';
		}
		
		/* Skip empty lines and comments */
		char *start = line;
		while (*start == ' ' || *start == '\t') {
			start++;
		}
		
		if (*start == '\0' || *start == '#') {
			continue;
		}
		
		/* Parse OID */
		oid_t oid;
		if (parse_oid_string(start, &oid) == 0) {
			/* Check for duplicates */
			int duplicate = 0;
			for (int i = 0; i < oid_list->count; i++) {
				if (compare_oids(&oid, &oid_list->oids[i]) == 0) {
					log_message(verbose_mode, "Warning: Duplicate OID %s on line %d\n", oid.str, line_number);
					duplicate = 1;
					break;
				}
			}
			
			if (!duplicate) {
				oid_list->oids[oid_list->count] = oid;
				oid_list->count++;
				log_message(verbose_mode, "Loaded OID: %s\n", oid.str);
			}
		} else {
			log_error("Invalid OID on line %d: %s\n", line_number, start);
			fclose(file);
			return -1;
		}
	}
	
	fclose(file);
	
	if (oid_list->count == 0) {
		log_error("No valid OIDs found in file '%s'\n", filename);
		return -1;
	}
	
	if (oid_list->count >= MAX_OID_COUNT) {
		log_message(verbose_mode, "Warning: Maximum OID limit (%d) reached\n", MAX_OID_COUNT);
	}
	
	return 0;
}

void print_oid(const oid_t *oid) {
	if (!oid) {
		return;
	}
	
	printf("%s", oid->str);
}

int compare_oids(const oid_t *oid1, const oid_t *oid2) {
	if (!oid1 || !oid2) {
		return -1;
	}
	
	/* Compare lengths */
	if (oid1->length != oid2->length) {
		return oid1->length - oid2->length;
	}
	
	/* Compare components */
	for (int i = 0; i < oid1->length; i++) {
		if (oid1->numbers[i] != oid2->numbers[i]) {
			if (oid1->numbers[i] < oid2->numbers[i]) {
				return -1;
			} else {
				return 1;
			}
		}
	}
	
	return 0;
}