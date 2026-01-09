# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# FSM Application paths
FSM_APP_DIR = ../../src
FSM_INCLUDE_DIR = $(FSM_APP_DIR)/headers
FSM_SRC_DIR = $(FSM_APP_DIR)/src

# FSM files
FSM_FILE = ../counter.fsm
FSM_NAME = CounterFSM
FSM_CALLBACK_BASE = counter_callback
FSM_GOTO_BASE = counter_goto

# Demo sources
DEMO_CALLBACK_SRC = demo_callback.cpp
DEMO_GOTO_SRC = demo_goto.cpp

# Output executables
TARGET_CALLBACK = fsm_demo_callback
TARGET_GOTO = fsm_demo_goto

# Object files per variant
OBJS_CALLBACK = $(BUILD_DIR)/$(DEMO_CALLBACK_SRC:.cpp=.o) \
                $(BUILD_DIR)/$(FSM_CALLBACK_BASE).o

OBJS_GOTO = $(BUILD_DIR)/$(DEMO_GOTO_SRC:.cpp=.o) \
            $(BUILD_DIR)/$(FSM_GOTO_BASE).o

# Default target: build both
all: directories generate_fsm $(TARGET_CALLBACK) $(TARGET_GOTO)

# Create directories
directories:
	mkdir -p $(BUILD_DIR)

# Generate FSM code using the FSM application
generate_fsm: $(FSM_FILE) build_generator
	@echo "Generating FSM code from $(FSM_FILE)..."
	$(BUILD_DIR)/generator $(FSM_FILE) $(FSM_NAME) $(FSM_CALLBACK_BASE) $(FSM_GOTO_BASE)

# Build the generator
build_generator: fsm_generator.cpp
	@echo "Building FSM generator..."
	$(CXX) $(CXXFLAGS) -I$(FSM_INCLUDE_DIR) -o $(BUILD_DIR)/generator fsm_generator.cpp \
	$(FSM_SRC_DIR)/machine_file_handler.cpp \
	$(FSM_SRC_DIR)/includable_generator.cpp \
	$(FSM_SRC_DIR)/moore_machine.cpp \
	$(FSM_SRC_DIR)/state.cpp \
	$(FSM_SRC_DIR)/transition.cpp \
	$(FSM_SRC_DIR)/expression_parser.cpp

# Build the callback version
$(TARGET_CALLBACK): $(OBJS_CALLBACK)
	@echo "Building $(TARGET_CALLBACK)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET_CALLBACK) $(OBJS_CALLBACK)

# Build the goto version
$(TARGET_GOTO): $(OBJS_GOTO)
	@echo "Building $(TARGET_GOTO)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET_GOTO) $(OBJS_GOTO)

# Compile the callback demo source
$(BUILD_DIR)/$(DEMO_CALLBACK_SRC:.cpp=.o): $(DEMO_CALLBACK_SRC) generate_fsm
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile the goto demo source
$(BUILD_DIR)/$(DEMO_GOTO_SRC:.cpp=.o): $(DEMO_GOTO_SRC) generate_fsm
	$(CXX) $(CXXFLAGS) -c -DUSE_COMPUTED_GOTO -g $< -o $@

# Compile the generated callback FSM code
$(BUILD_DIR)/$(FSM_CALLBACK_BASE).o: $(FSM_CALLBACK_BASE).cpp $(FSM_CALLBACK_BASE).h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile the generated goto FSM code with computed goto flag
$(BUILD_DIR)/$(FSM_GOTO_BASE).o: $(FSM_GOTO_BASE).cpp $(FSM_GOTO_BASE).h
	$(CXX) $(CXXFLAGS) -O0 -g -DUSE_COMPUTED_GOTO -c $< -o $@

# Clean the build
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET_CALLBACK) $(TARGET_GOTO)
	rm -f $(FSM_CALLBACK_BASE).cpp $(FSM_CALLBACK_BASE).h
	rm -f $(FSM_GOTO_BASE).cpp $(FSM_GOTO_BASE).h

# Clean everything including the generated FSM code
clean_all: clean
	rm -f $(BUILD_DIR)/generator

# Run the callback version
run_callback: $(TARGET_CALLBACK)
	./$(TARGET_CALLBACK)

# Run the goto version
run_goto: $(TARGET_GOTO)
	./$(TARGET_GOTO)

.PHONY: all clean clean_all run_callback run_goto directories generate_fsm build_generator