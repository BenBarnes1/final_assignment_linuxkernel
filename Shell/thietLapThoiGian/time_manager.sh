#!/bin/bash

COMMAND="$1"
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${BASE_DIR}/log/time_manager.log"

# Tự động tạo thư mục log nếu chưa tồn tại
mkdir -p "${BASE_DIR}/log"

E_NOTROOT=87
E_INVALID=88

# Hàm ghi log súc tích, sạch sẽ theo quy tắc thiết kế
log_info() {
    local msg="[INFO] $(date +'%Y-%m-%d %H:%M:%S') - $1"
    echo "$msg" >> "$LOG_FILE"
    echo "$msg"
}

log_error() {
    local msg="[ERROR] $(date +'%Y-%m-%d %H:%M:%S') - $1"
    echo "$msg" >> "$LOG_FILE"
    echo "$msg" >&2
}

# Kiểm tra quyền Root bằng UID hệ thống (Chặt chẽ hơn lệnh whoami)
check_root() {
    if [ "$(id -u)" -ne 0 ]; then
        log_error "Lỗi: Bạn cần quyền root để sử dụng chức năng này. Vui lòng chạy với 'sudo'."
        exit $E_NOTROOT
    fi
}

# Chức năng thay đổi giờ (chhour)
change_hour() {
    check_root
    read -p "Nhập giờ mới theo định dạng hh:mm:ss (VD: 14:30:00): " new_time

    # Xử lý ngoại lệ bằng Regex: Kiểm tra định dạng chuẩn từ 00:00:00 đến 23:59:59
    if [[ ! "$new_time" =~ ^([0-1][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$ ]]; then
        log_error "Định dạng giờ không hợp lệ ($new_time). Thao tác bị từ chối."
        exit $E_INVALID
    fi

    # Thực hiện thay đổi giờ (Cơ chế Try-Catch)
    if date +%T -s "$new_time" >/dev/null 2>&1; then
        log_info "Thay đổi giờ hệ thống thành công thành: $new_time"
    else
        log_error "Lỗi hệ thống: Không thể áp dụng thay đổi giờ (Có thể do xung đột NTP)."
    fi
}

# Chức năng thay đổi ngày (chdate)
change_date() {
    check_root
    read -p "Nhập ngày mới theo định dạng yyyymmdd (VD: 20260617): " new_date

    # Xử lý ngoại lệ bằng Regex: Kiểm tra đầu vào phải là cụm 8 chữ số
    if [[ ! "$new_date" =~ ^[0-9]{8}$ ]]; then
        log_error "Định dạng ngày không hợp lệ ($new_date). Phải là 8 chữ số liền nhau."
        exit $E_INVALID
    fi

    # Tách chuỗi để phục vụ kiểm tra biên nâng cao
    local year="${new_date:0:4}"
    local month="${new_date:4:2}"
    local day="${new_date:6:2}"

    # Bắt lỗi biên (Edge Case): Kiểm tra tính logic của ngày (Ví dụ loại bỏ ngày 31/02 hoặc ngày 32/12)
    if ! date -d "$year-$month-$day" >/dev/null 2>&1; then
        log_error "Ngày nhập vào không tồn tại trong thực tế: Ngày $day tháng $month năm $year."
        exit $E_INVALID
    fi

    # Thực hiện thay đổi ngày (Cơ chế Try-Catch)
    if date -s "$new_date" >/dev/null 2>&1; then
        log_info "Thay đổi ngày hệ thống thành công."
        date +"Kết quả áp dụng -> Năm: %Y Tháng: %m Ngày: %d"
    else
        log_error "Lỗi hệ thống: Không thể cấu hình lại ngày."
    fi
}

# Chức năng bổ sung: Xem thời gian hiện tại
view_time() {
    echo "--- Thời gian hệ thống hiện hành ---"
    date +"Bây giờ là: %H:%M:%S | Ngày: %d-%m-%Y"
}

# Điều hướng kịch bản chính
case "$COMMAND" in
    chhour)
        change_hour
        ;;
    chdate)
        change_date
        ;;
    view)
        view_time
        ;;
    *)
        echo "Sử dụng cú pháp trực diện: $0 {chhour|chdate|view}"
        exit 1
        ;;
esac
