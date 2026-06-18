#!/bin/bash

INPUT_FILE="$1"
LOG_DIR="./log"
SUCCESS_LOG="${LOG_DIR}/nenhoang_install_success.log"
ERROR_LOG="${LOG_DIR}/nenhoang_install_error.log"

# Đảm bảo thư mục lưu log tồn tại
mkdir -p "$LOG_DIR"

# Cấu hình cài đặt im lặng, chặn mọi giao diện cấu hình tương tác
export DEBIAN_FRONTEND=noninteractive

# Các hàm ghi log chuẩn hóa theo cấu trúc định dạng yêu cầu
log_success() {
    local app="$1"
    local status="$2"
    local msg="$3"
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [$status] [$app] - $msg" >> "$SUCCESS_LOG"
    echo "[$status] [$app] - $msg"
}

log_error_raw() {
    local app="$1"
    local status="$2"
    local msg="$3"
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [$status] [$app] - $msg" >> "$ERROR_LOG"
    echo "[$status] [$app] - $msg" >&2
}

log_error_structured() {
    local app="$1"
    local reason="$2"
    local fix="$3"
    
    {
        echo "--------------------------------------------------"
        echo "Thời gian: $(date +'%Y-%m-%d %H:%M:%S')"
        echo "Ứng dụng: $app"
        echo "Trạng thái: Thất bại"
        echo "Nguyên nhân: $reason"
        echo "Khắc phục sự cố (Hướng dẫn hành động): $fix"
        echo "--------------------------------------------------"
    } >> "$ERROR_LOG"
}

# 1. Kiểm tra quyền root
check_root() {
    if [ "$(id -u)" -ne 0 ]; then
        echo "[ERROR] Lỗi hệ thống: Bạn phải chạy script này bằng quyền root (sudo)." >&2
        exit 87
    fi
}

# 2. Kiểm tra kết nối Internet
check_internet() {
    if ! ping -c 1 -W 2 8.8.8.8 &>/dev/null && ! ping -c 1 -W 2 archive.ubuntu.com &>/dev/null; then
        echo "[ERROR] Lỗi hệ thống: Không có kết nối Internet. Kiểm tra lại đường truyền." >&2
        exit 89
    fi
}

# 3. Kiểm tra dung lượng ổ cứng phân vùng root (Yêu cầu trống >= 2GB)
check_disk_space() {
    local free_space
    free_space=$(df / --output=avail | tail -n1) # Trả về đơn vị KB
    local free_gb=$((free_space / 1024 / 1024))
    if [ "$free_gb" -lt 2 ]; then
        echo "[ERROR] Lỗi hệ thống: Dung lượng đĩa trống trên phân vùng root (/) hiện tại là ${free_gb}GB, nhỏ hơn mức tối thiểu yêu cầu (2GB)." >&2
        exit 88
    fi
}

# 4. Kiểm tra và giải phóng APT Lock (Chờ tối đa 3 phút)
check_apt_lock() {
    local count=0
    while [ $count -lt 36 ]; do # 36 lần * 5 giây = 180 giây
        if fuser /var/lib/apt/lists/lock /var/lib/dpkg/lock-frontend /var/lib/dpkg/lock >/dev/null 2>&1; then
            echo "[WARN] Phát hiện tiến trình khác đang chiếm giữ APT/Dpkg Lock. Đang chờ giải phóng (Thử lại sau 5s)..."
            sleep 5
            count=$((count+1))
        else
            return 0
        fi
    done
    echo "[ERROR] Lỗi hệ thống: APT lock bị khóa quá lâu bởi tiến trình khác. Thoát để đảm bảo an toàn." >&2
    exit 90
}

# --- BỘ PHÂN TÍCH STDERR VÀ TRÍCH XUẤT NGUYÊN NHÂN LỖI ---
analyze_and_log_failure() {
    local app="$1"
    local err_file="$2"
    local err_content
    err_content=$(cat "$err_file" 2>/dev/null)
    
    local reason="Lỗi chưa xác định trong tiến trình apt-get"
    local fix="Thử chạy lệnh 'sudo dpkg --configure -a' để kiểm tra lại tính toàn vẹn của hệ thống cấu hình."

    if echo "$err_content" | grep -q -i "Unable to locate package"; then
        reason="Không tìm thấy package '$app' trong kho lưu trữ hiện tại (Sai chính tả tên gói)."
        fix="Vui lòng kiểm tra lại chính tả tên package hoặc cần cấu hình thêm repository PPA phù hợp cho gói phần mềm này."
    elif echo "$err_content" | grep -q -i "Depends:"; then
        reason="Hỏng hoặc xung đột cấu trúc Dependency (Gói phụ thuộc liên quan bị lỗi)."
        fix="Thực hiện gỡ bỏ package xung đột hoặc chạy lệnh 'sudo apt-get install -f' một cách độc lập để làm sạch."
    elif echo "$err_content" | grep -q -iE "404  Not Found|Could not connect|Temporary failure"; then
        reason="Mất kết nối cục bộ hoặc máy chủ kho lưu trữ Ubuntu phản hồi lỗi mạng 404."
        fix="Vui lòng thực hiện lệnh 'sudo apt-get update' trước khi chạy hoặc kiểm tra lại cấu hình DNS/Proxy của hệ thống."
    fi

    log_error_raw "$app" "ERROR" "Cài đặt thất bại sau 3 lần thử. Đã phân tích nguyên nhân lỗi vào file log."
    log_error_structured "$app" "$reason" "$fix"
}

# --- TIẾN TRÌNH CÀI ĐẶT CHÍNH ---
install_process() {
    if [ -z "$INPUT_FILE" ] || [ ! -f "$INPUT_FILE" ]; then
        echo "[ERROR] Sử dụng cú pháp: $0 {đường_dẫn_file_danh_sách}" >&2
        exit 1
    fi

    # Thực thi các Pre-flight kiểm tra an toàn hệ thống
    check_root
    check_internet
    check_disk_space
    check_apt_lock

    # Đọc tệp danh sách, bỏ qua dòng trống và dòng comment bắt đầu bằng dấu #
    while IFS= read -r line || [ -n "$line" ]; do
        # Xóa khoảng trắng thừa ở đầu/cuối dòng
        local app
        app=$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        
        # Bỏ qua dòng trống hoặc dòng comment
        [[ -z "$app" || "$app" =~ ^# ]] && continue

        # Tính bất biến (Idempotency): Kiểm tra ứng dụng đã tồn tại chưa
        if dpkg -l "$app" 2>/dev/null | grep -q "^ii"; then
            log_success "$app" "INFO" "Đã tồn tại trên hệ thống. Bỏ qua."
            continue
        fi

        local success=false
        local attempt=1
        local max_retries=3
        local tmp_err_file
        tmp_err_file=$(mktemp)

        while [ $attempt -le $max_retries ]; do
            log_success "$app" "INFO" "Đang tiến hành cài đặt (Lần thử: $attempt/$max_retries)..."
            
            # Thực thi cài đặt, gom stderr lại để phân tích nếu lỗi
            if apt-get install -y "$app" >/dev/null 2>"$tmp_err_file"; then
                log_success "$app" "SUCCESS" "Cài đặt thành công vào hệ thống."
                success=true
                break
            else
                log_error_raw "$app" "WARN" "Lần thử thứ $attempt thất bại. Kích hoạt cơ chế tự vá lỗi phụ thuộc..."
                apt-get install -f -y >/dev/null 2>&1
                attempt=$((attempt + 1))
                [ $attempt -le $max_retries ] && sleep 5
            fi
        done

        # Nếu qua 3 lần thử vẫn lỗi, xử lý phân tích và chuyển tiếp sang app khác
        if [ "$success" = false ]; then
            analyze_and_log_failure "$app" "$tmp_err_file"
        fi

        rm -f "$tmp_err_file"
    done < "$INPUT_FILE"
}

# Kích hoạt chương trình
install_process
