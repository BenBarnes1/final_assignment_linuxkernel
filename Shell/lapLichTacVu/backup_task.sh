#!/bin/bash
# Tự động lấy thư mục hiện tại làm gốc
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SOURCE_DIR="${BASE_DIR}/source"
BACKUP_BASE_DIR="${BASE_DIR}/backup"
LOG_FILE="${BASE_DIR}/log/backup_log.txt"

# Định dạng ngày giờ chạy
CURRENT_DATE=$(date +'%Y-%m-%d')
CURRENT_TIME=$(date +'%Y-%m-%d %H:%M:%S')
BACKUP_DIR="${BACKUP_BASE_DIR}/${CURRENT_DATE}"

# Luôn đảm bảo thư mục log tồn tại trước khi ghi
mkdir -p "${BASE_DIR}/log"

log() {
    echo "$CURRENT_TIME - $1" >> "$LOG_FILE"
}

# Kiểm tra thư mục nguồn có tồn tại không
if [ ! -d "$SOURCE_DIR" ]; then
    log "[ERROR] Thư mục nguồn không tồn tại: $SOURCE_DIR. Dừng sao lưu."
    exit 1
fi

# Tạo thư mục đích an toàn
if ! mkdir -p "$BACKUP_DIR"; then
    log "[ERROR] Không thể tạo thư mục đích: $BACKUP_DIR."
    exit 1
fi

# Thực hiện sao lưu và bắt lỗi dữ liệu trống hoặc lỗi sao chép
if [ -z "$(ls -A "$SOURCE_DIR" 2>/dev/null)" ]; then
    log "[WARN] Thư mục nguồn trống. Không có tệp nào được sao lưu."
    exit 0
fi

if cp -r "$SOURCE_DIR"/* "$BACKUP_DIR" 2>/dev/null; then
    log "[SUCCESS] Hoàn thành sao lưu từ $SOURCE_DIR sang $BACKUP_DIR"
else
    log "[ERROR] Gặp sự cố trong quá trình sao chép dữ liệu."
    exit 1
fi