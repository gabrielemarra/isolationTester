# Makefile for compiling 'isolation_tester'

# Compiler settings - can change cc and CFLAGS
CC=g++
CFLAGS=-Wall -g

# Target executable name
TARGET=isolation_tester

# Source files
SRC=isolation_tester.cpp

# Library to link
LIBS=-lcap

# Default target
all: $(TARGET)

# Rule for building the target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Rule for cleaning up
clean:
	rm -f $(TARGET)

# Rule to compile and run
run: $(TARGET)
	./$(TARGET)
