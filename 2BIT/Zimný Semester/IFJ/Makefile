# Compiler and flags
CC = gcc
#CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -DDEBUG -Ofast
CFLAGS = -std=c99 -Ofast -pedantic -w
#CFLAGS = -O3 -Ofast -march=native -funroll-loops -fomit-frame-pointer -flto -ffast-math -fno-exceptions -fstrict-aliasing -fno-threadsafe-statics -g0

# Target executable
TARGET = scanner

# Source files
SRCS = main.c scanner.c ptr_registry.c parser.c error.c symtable.c generator.c dynamic_string.c label_list.c
OBJS = $(SRCS:.c=.o)

DOC_FILE = dokumentace.odt
PDF_FILE = dokumentace.pdf

# Default test sample code argument value
FILE ?= test.in

# Output of test is saved to output file
OUTFILE = $(FILE:.in=.out)
DEBUGFILE = $(OUTFILE).debug

# Default target: run 'clean' first, then build the program
all: clean $(TARGET)

# Rule to link the object files and create the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean: remove object files and the target executable
clean:
	rm -f $(OBJS) $(TARGET)

clean_tests:
	rm -f test_samples/*.out test_samples/*.debug

# Run: builds the program and runs it with the FILE argument
# Also saves the debug output right now
run: clean $(TARGET)
	rm -f $(OBJS)
	./$(TARGET) $(FILE)
# > $(OUTFILE) 2> $(DEBUGFILE)

# Target to convert .odt to .pdf
$(PDF_FILE): $(DOC_FILE)
	libreoffice --headless --convert-to pdf $<

pack: $(PDF_FILE)
	zip xjenisf00.zip *.c *.h Makefile dokumentace.pdf rozdeleni rozsireni

