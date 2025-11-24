# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc -Iinclude
LDFLAGS = -lcurl -ljson-c -lncurses

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .

# Target executable
TARGET = $(BIN_DIR)/spotCLI
# TARGET = $(BIN_DIR)/spoTUI

# Find all source files recursively
SOURCES = $(shell find $(SRC_DIR) -type f -name '*.c')
# Convert source paths to object paths in build directory
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
# Dependency files
DEPS = $(OBJECTS:.o=.d)

# Colors for output
COLOR_RESET = \033[0m
COLOR_GREEN = \033[38;2;67;251;43m
COLOR_YELLOW = \033[38;2;255;192;0m
COLOR_BLUE = \033[38;2;43;123;251m

# Default target
.PHONY: all
all: $(TARGET)
	@echo "$(COLOR_GREEN)✓ Build complete!$(COLOR_RESET)"
	@echo "$(COLOR_BLUE)Run with: ./spotCLI$(COLOR_RESET)"
	@echo "$(COLOR_BLUE)Or install with: sudo make install$(COLOR_RESET)"

# Ensure build directories exist
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "$(COLOR_YELLOW)Compiling $<...$(COLOR_RESET)"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Link objects
$(TARGET): $(OBJECTS)
	@echo "$(COLOR_BLUE)Linking $(TARGET)...$(COLOR_RESET)"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Include dependency files (if they exist)
-include $(DEPS)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "$(COLOR_YELLOW)Cleaning build files...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "$(COLOR_GREEN)✓ Clean complete!$(COLOR_RESET)"

# Clean and rebuild
.PHONY: rebuild
rebuild: clean all

# Run the program
.PHONY: run
run: $(TARGET)
	@echo "$(COLOR_BLUE)Running spotCLI...$(COLOR_RESET)"
	@./$(TARGET)

# Install to system (requires sudo)
.PHONY: install
install: $(TARGET)
	@echo "$(COLOR_BLUE)Installing spotCLI to /usr/local/bin...$(COLOR_RESET)"
	@sudo cp $(TARGET) /usr/local/bin/spotCLI
	@sudo chmod +x /usr/local/bin/spotCLI
	@echo "$(COLOR_GREEN)✓ Installation complete!$(COLOR_RESET)"

# Uninstall
.PHONY: uninstall
uninstall:
	@echo "$(COLOR_YELLOW)Uninstalling spotCLI...$(COLOR_RESET)"
	@sudo rm -f /usr/local/bin/spotCLI
	@echo "$(COLOR_GREEN)✓ Uninstall complete!$(COLOR_RESET)"

# Logout (remove token)
.PHONY: logout
logout:
	@echo "$(COLOR_YELLOW)Removing authentication token...$(COLOR_RESET)"
	@rm -f ~/.config/spotCLI/token.json
	@echo "$(COLOR_GREEN)✓ Logged out!$(COLOR_RESET)"

# Debug build with symbols
.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "$(COLOR_GREEN)✓ Debug build complete!$(COLOR_RESET)"

# Check for ncurses
.PHONY: check-deps
check-deps:
	@echo "$(COLOR_BLUE)Checking dependencies...$(COLOR_RESET)"
	@which pkg-config > /dev/null || (echo "$(COLOR_YELLOW)pkg-config not found$(COLOR_RESET)" && exit 1)
	@pkg-config --exists ncurses || (echo "$(COLOR_YELLOW)ncurses not found. Install with: sudo apt install libncurses5-dev$(COLOR_RESET)" && exit 1)
	@pkg-config --exists libcurl || (echo "$(COLOR_YELLOW)libcurl not found$(COLOR_RESET)" && exit 1)
	@pkg-config --exists json-c || (echo "$(COLOR_YELLOW)json-c not found$(COLOR_RESET)" && exit 1)
	@echo "$(COLOR_GREEN)✓ All dependencies found!$(COLOR_RESET)"

# Help
.PHONY: help
help:
	@echo "$(COLOR_BLUE)spotCLI Makefile Commands:$(COLOR_RESET)"
	@echo ""
	@echo "  $(COLOR_GREEN)make$(COLOR_RESET)            - Build the project with ncurses support"
	@echo "  $(COLOR_GREEN)make check-deps$(COLOR_RESET) - Check if all dependencies are installed"
	@echo "  $(COLOR_GREEN)make clean$(COLOR_RESET)      - Remove build files"
	@echo "  $(COLOR_GREEN)make rebuild$(COLOR_RESET)    - Clean and rebuild"
	@echo "  $(COLOR_GREEN)make run$(COLOR_RESET)        - Build and run"
	@echo "  $(COLOR_GREEN)make debug$(COLOR_RESET)      - Build with debug symbols"
	@echo "  $(COLOR_GREEN)make install$(COLOR_RESET)    - Install to /usr/local/bin (requires sudo)"
	@echo "  $(COLOR_GREEN)make uninstall$(COLOR_RESET)  - Remove from /usr/local/bin"
	@echo "  $(COLOR_GREEN)make logout$(COLOR_RESET)     - Remove authentication token"
	@echo "  $(COLOR_GREEN)make help$(COLOR_RESET)       - Show this help message"
	@echo ""
	@echo "$(COLOR_BLUE)Dependencies required:$(COLOR_RESET)"
	@echo "  - ncurses (libncurses5-dev)"
	@echo "  - libcurl (libcurl4-openssl-dev)"
	@echo "  - json-c (libjson-c-dev)"
	@echo ""
