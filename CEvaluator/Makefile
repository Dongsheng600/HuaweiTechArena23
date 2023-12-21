# Makefile

# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -O2

# Name of the final executable
TARGET = main

# Default target
all: $(TARGET)

# Linking the final executable
$(TARGET): main.o predict.o
	$(CC) $(CFLAGS) main.o predict.o -o $(TARGET)

# Compiling main.c to main.o
main.o: main.c predict.h
	$(CC) $(CFLAGS) -c main.c

# Compiling predict.c to predict.o
predict.o: predict.c predict.h
	$(CC) $(CFLAGS) -c predict.c

# Clean target for removing compiled files
clean:
	rm -f $(TARGET) main.o predict.o