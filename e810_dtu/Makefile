# Makefile for E810-DTU Test Tool
# Senior-level: Clean, modular, well-documented

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2
LDFLAGS =

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/network.c \
          $(SRC_DIR)/modbus_client.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/test-e810

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

# Install to system
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/test-e810

# Run tests
test: $(TARGET)
	./$(TARGET) --help

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

.PHONY: all clean install uninstall test debug directories