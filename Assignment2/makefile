# Define the C compiler and flags
CC = gcc
CFLAGS = -g -Wall

# The build target executable
TARGET = threaded_sum

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET)