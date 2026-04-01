#/*******************************************************************************
#*                                                                              *
#*                        Brno University of Technology                         *
#*                      Faculty of Information Technology                       *
#*                                                                              *
#*               Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)             *
#*                                                                              *
#*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
#*                                   Brno 2025                                  *
#*                                                                              *
#*                                   Makefile                                   *
#*                                                                              *
#*******************************************************************************/

CC = gcc

CFLAGS = -D_XOPEN_SOURCE=700 -Wall -Wextra -std=c99 -pedantic -Werror -Wconversion -Wsign-conversion -Wdouble-promotion -Wpadded -Wpacked -Wunsafe-loop-optimizations #-Wstack-usage=1024
LDFLAGS = 
LIBS = 

# Explicitly list object files for BSD make compatibility
OBJECTS = main.o oid.o snmp.o http.o otel.o utils.o
TARGET = snmp2otel

# Test object files
TEST_OBJECTS = test_oid.o test_snmp.o test_http.o
TEST_TARGET = test_snmp2otel
TEST_RUNNER = test_runner

# Default target
all: $(TARGET)

# Build main program
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

# Build object files
main.o: main.c snmp2otel.h
	$(CC) $(CFLAGS) -c main.c -o main.o

oid.o: oid.c snmp2otel.h
	$(CC) $(CFLAGS) -c oid.c -o oid.o

snmp.o: snmp.c snmp2otel.h
	$(CC) $(CFLAGS) -c snmp.c -o snmp.o

http.o: http.c snmp2otel.h
	$(CC) $(CFLAGS) -c http.c -o http.o

otel.o: otel.c snmp2otel.h
	$(CC) $(CFLAGS) -c otel.c -o otel.o

utils.o: utils.c snmp2otel.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

# Build unit tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): oid.o snmp.o http.o otel.o utils.o $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TEST_TARGET) oid.o snmp.o http.o otel.o utils.o $(TEST_OBJECTS) $(LIBS)

test_oid.o: test_oid.c snmp2otel.h
	$(CC) $(CFLAGS) -c test_oid.c -o test_oid.o

test_snmp.o: test_snmp.c snmp2otel.h
	$(CC) $(CFLAGS) -c test_snmp.c -o test_snmp.o

test_http.o: test_http.c snmp2otel.h
	$(CC) $(CFLAGS) -c test_http.c -o test_http.o

# Build all tests
$(TEST_RUNNER): test_runner.c
	$(CC) $(CFLAGS) -o $(TEST_RUNNER) test_runner.c

# Run all tests
test-all: $(TEST_RUNNER)
	./$(TEST_RUNNER)

# Run the program with example parameters
run: $(TARGET)
	@echo "Usage: ./$(TARGET) -t target -o oids_file -e endpoint [-C community] [-i interval] [-r retries] [-T timeout] [-p port] [-v]"
	@echo "Example: ./$(TARGET) -t 192.168.1.1 -o oids.txt -e http://localhost:4318/v1/metrics -v"

# Create example files
examples: oids.txt

oids.txt:
	@echo "# Example OID file for snmp2otel" > oids.txt
	@echo "# System uptime" >> oids.txt
	@echo "1.3.6.1.2.1.1.3.0" >> oids.txt
	@echo "# System description" >> oids.txt
	@echo "1.3.6.1.2.1.1.1.0" >> oids.txt
	@echo "# System name" >> oids.txt
	@echo "1.3.6.1.2.1.1.5.0" >> oids.txt
	@echo "Created example oids.txt file"

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET) $(TEST_RUNNER)

# Clean everything including examples
distclean: clean
	rm -f oids.txt

# Show program information
info:
	@echo "SNMP to OpenTelemetry Exporter (snmp2otel)"
	@echo "=========================================="
	@echo "Object files: $(OBJECTS)"
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Target: $(TARGET)"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build the main program"
	@echo "  test      - Build and run unit tests"
	@echo "  test-all  - Build and run comprehensive test suite"
	@echo "  run       - Show example usage"
	@echo "  examples  - Create example configuration files"
	@echo "  clean     - Remove build artifacts"
	@echo "  info      - Show this information"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)

# Static analysis (available only on eva)
analyze:
	@which cppcheck >/dev/null 2>&1 && cppcheck --enable=warning,style,performance,portability --check-level=exhaustive --std=c99 main.c oid.c snmp.c http.c otel.c utils.c || echo "cppcheck not available"

# Memory check (if available)
memcheck: $(TARGET)
	@which valgrind >/dev/null 2>&1 && echo "Run: valgrind --leak-check=full ./$(TARGET) [args]" || echo "valgrind not available"

# Package for submission
package: clean
	@echo "Creating submission package..."
	@tar -cf xbohach00.tar *.c *.h Makefile README oids.txt manual.pdf
	@echo "Package created: xbohach00.tar"
	@echo "Contents:"
	@tar -tf xbohach00.tar

# Help target
help: info

.PHONY: all test test-all run clean distclean info debug analyze memcheck help examples package