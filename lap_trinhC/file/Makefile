CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = file_manager
SRCS = file_manager.c

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "[BUILD] Bien dich $(SRCS)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
	@echo "[BUILD] Hoan tat. Chay lenh './$(TARGET)' de bat dau."
	
run:
	./$(TARGET)
	
clean:
	@echo "[CLEAN] Xoa cac file thuc thi..."
	rm -f $(TARGET)
