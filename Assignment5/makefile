# Define the C compiler and flags
CC = gcc
CFLAGS = -g -Wall -pedantic

# The build target executable
TARGET = mymalloc

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET)