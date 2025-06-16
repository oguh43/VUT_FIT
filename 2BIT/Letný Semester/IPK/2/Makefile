# xbohach00
# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c17 #-Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition

CFLAGS += -DDEBUG_PRINT

LDFLAGS =
# -lrt

SRC_DIR = src
OBJ_DIR = obj
NODEBUG_OBJ_DIR = obj_nodebug
BIN_DIR = .

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
NODEBUG_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(NODEBUG_OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/ipk25chat-client
NODEBUG_TARGET = $(BIN_DIR)/ipk25chat-client-nodebug

.PHONY: all clean nodebug

all: $(TARGET)

# Target without debugging
nodebug: CFLAGS := $(filter-out -DDEBUG_PRINT,$(CFLAGS))
nodebug: $(NODEBUG_TARGET)
	@echo "Built without debug output: $(NODEBUG_TARGET)"
	@cp $(NODEBUG_TARGET) $(TARGET)
	@echo "Copied to $(TARGET)"

$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(NODEBUG_TARGET): $(NODEBUG_OBJ_FILES) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NODEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(NODEBUG_OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR) $(OBJ_DIR) $(NODEBUG_OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(NODEBUG_OBJ_DIR) $(TARGET) $(NODEBUG_TARGET)