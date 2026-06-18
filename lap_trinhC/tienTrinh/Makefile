CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = process_master
SRC = process_master.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean run
