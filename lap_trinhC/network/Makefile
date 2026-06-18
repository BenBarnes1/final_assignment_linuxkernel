CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = network_manager
SRCS = network_manager.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
