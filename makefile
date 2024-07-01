# Makefile for limn
# Compiler
CC = clang

# Directories
BINDIR = ./bin

# Target executable
TARGET = $(BINDIR)/limn

# Source file
SRC = limn.c

# Compiler flags
CFLAGS = -std=c11 -O2 -g -Wall -Wextra -Wpedantic -Werror \
         -fstack-protector-strong -fPIE -D_FORTIFY_SOURCE=2 \
         -fsanitize=undefined -fno-sanitize-recover=all \
         -fvisibility=hidden

# Linker flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -Wl,-z,relro,-z,now
else
    LDFLAGS =
endif

# Libraries
LIBS = $(shell pkg-config --libs raylib)

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LIBS) -o $(TARGET)

# Create bin directory if it doesn't exist
$(BINDIR):
	mkdir -p $(BINDIR)

# Clean rule
clean:
	rm -f $(TARGET)
	rmdir $(BINDIR) 2>/dev/null || true

.PHONY: all clean
