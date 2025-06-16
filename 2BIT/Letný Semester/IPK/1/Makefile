CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lpcap

TARGET = ipk-l4-scan

SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/scanner.c $(SRC_DIR)/tcp_scanner.c $(SRC_DIR)/udp_scanner.c $(SRC_DIR)/util.c $(SRC_DIR)/tcp_scanner_ipv6.c $(SRC_DIR)/udp_scanner_ipv6.c
HEADERS = $(SRC_DIR)/scanner.h $(SRC_DIR)/util.h

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)


.PHONY: all clean