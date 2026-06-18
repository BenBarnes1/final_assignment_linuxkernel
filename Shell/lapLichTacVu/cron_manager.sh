#!/bin/bash
COMMAND="$1"
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${BASE_DIR}/log/cron_manager.log"

# Tự động tạo thư mục log nếu chưa có
mkdir -p "${BASE_DIR}/log"

# Hàm ghi log cơ bản
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

# CHỨC NĂNG TẠO MỚI TÁC VỤ (ĐÃ NÂNG CẤP THÔNG MINH)
create_task() {
    echo "--- Khởi tạo Tác vụ Mới ---"
    echo "Gợi ý các file kịch bản (.sh) có sẵn trong thư mục hiện tại:"
    ls *.sh 2>/dev/null | sed 's/^/  - /'
    echo ""

    read -p "Nhập tên file tác vụ (VD: backup_task.sh): " file_name
    
    if [ -z "$file_name" ]; then
        log_error "Tên file tác vụ không được để trống. Hủy thao tác."
        exit 1
    fi

    # TỰ ĐỘNG BIẾN THÀNH ĐƯỜNG DẪN TUYỆT ĐỐI
    local cmd="${BASE_DIR}/${file_name}"

    # Kiểm tra biên: File có tồn tại không?
    if [ ! -f "$cmd" ]; then
        log_error "Không tìm thấy file '$file_name' trong thư mục hiện tại (${BASE_DIR})."
        exit 1
    fi

    # Kiểm tra biên: File đã có quyền thực thi chưa? Nếu chưa thì tự cấp quyền
    if [ ! -x "$cmd" ]; then
        log_info "Phát hiện file chưa có quyền thực thi. Hệ thống tự động cấp quyền 'chmod +x'..."
        chmod +x "$cmd"
    fi

    log_info "Đường dẫn tuyệt đối được tự động thiết lập: $cmd"

    # Tiến hành cấu hình thời gian chạy
    echo "Thiết lập thời gian chạy (Để trống sẽ mặc định là '*')"
    read -p "Nhập phút (0-59): " min
    read -p "Nhập giờ (0-23): " hour
    read -p "Nhập ngày trong tháng (1-31): " day
    read -p "Nhập tháng (1-12): " month
    read -p "Nhập ngày trong tuần (0-6, Chủ nhật = 0): " dow

    min=${min:-*}
    hour=${hour:-*}
    day=${day:-*}
    month=${month:-*}
    dow=${dow:-*}

    read -p "Xác nhận tạo tác vụ với cấu hình: '$min $hour $day $month $dow' (Y/N)? " confirm
    if [[ "$confirm" =~ ^[Yy]$ ]]; then
        tmp_cron=$(mktemp)
        crontab -l > "$tmp_cron" 2>/dev/null
        
        if grep -q "$cmd" "$tmp_cron" 2>/dev/null; then
            log_info "Tác vụ này đã tồn tại trong hệ thống lập lịch từ trước."
            rm -f "$tmp_cron"
            exit 0
        fi

        echo "$min $hour $day $month $dow $cmd" >> "$tmp_cron"
        
        if crontab "$tmp_cron"; then
            log_info "Tạo mới tác vụ thành công."
        else
            log_error "Lỗi hệ thống: Không thể áp dụng crontab."
        fi
        rm -f "$tmp_cron"
    else
        log_info "Đã hủy thao tác tạo tác vụ."
    fi
}

# Chức năng liệt kê tác vụ (GIỮ NGUYÊN)
list_tasks() {
    echo "--- Danh sách tác vụ hiện tại ---"
    if crontab -l >/dev/null 2>&1; then
        crontab -l
        log_info "Truy xuất danh sách tác vụ thành công."
    else
        log_info "Hệ thống hiện chưa có tác vụ lập lịch nào."
    fi
}

# Chức năng xoá tác vụ (GIỮ NGUYÊN)
delete_tasks() {
    read -p "CẢNH BÁO: Thao tác này sẽ XÓA TOÀN BỘ các tác vụ lập lịch hiện tại. Tiếp tục (Y/N)? " confirm
    if [[ "$confirm" =~ ^[Yy]$ ]]; then
        if crontab -r 2>/dev/null; then
            log_info "Xóa toàn bộ tác vụ thành công."
        else
            log_error "Không có tác vụ nào để xóa hoặc lỗi quyền truy cập."
        fi
    else
        log_info "Đã hủy thao tác xóa."
    fi
}

# Điều hướng chính (GIỮ NGUYÊN)
case "$COMMAND" in
    create)
        create_task
        ;;
    list)
        list_tasks
        ;;
    delete)
        delete_tasks
        ;;
    *)
        echo "Sử dụng: $0 {create|list|delete}"
        exit 1
        ;;
esac