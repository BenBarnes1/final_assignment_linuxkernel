CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
SERVER_TARGET = server
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): server.c B2.h
	@echo "[BUILD] Bien dich Server..."
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) server.c

$(CLIENT_TARGET): client.c B2.h
	@echo "[BUILD] Bien dich Client..."
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) client.c

clean:
	@echo "[CLEAN] Xoa cac file thuc thi..."
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
