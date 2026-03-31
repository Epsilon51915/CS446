# Define the C compiler and flags
CC = gcc
CFLAGS = -g -Wall -pthread

# The build target executable
TARGET = sched

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET)