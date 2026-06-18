# Module: Tự động Cài đặt Ứng dụng Hàng loạt (bulk_installer.sh)

Hệ thống kịch bản nâng cao hỗ trợ quản trị viên/DevOps cài đặt danh sách ứng dụng một cách an toàn, có khả năng tự phục hồi trạng thái hệ thống, tự phân tích lỗi và cách ly ứng dụng lỗi mà không gây treo script.

## 1. Các điểm kỹ thuật cốt lõi trong xử lý biên

* **Pre-flight Checks:** Tự động bảo vệ hệ thống thông qua việc quét đặc quyền Root, kiểm tra mạng sống (Internet), đo dung lượng lưu trữ tối thiểu (>= 5GB) và lập vòng lặp 3 phút chờ giải phóng APT/Dpkg locks nếu có tiến trình cài đặt khác chạy ngầm.
* **Cơ chế Idempotency & Tự vá lỗi:** * Sử dụng định dạng bộ lọc `dpkg -l` để bỏ qua các ứng dụng đã cấu hình hoàn chỉnh.
  * Nếu lệnh `apt-get` lỗi, hệ thống tự động gọi hàm `apt-get install -f -y` nhằm đồng bộ thư viện và thử lại (Retry) tối đa 3 lần với khoảng thời gian nghỉ 5 giây.
* **Tách biệt Log & Trích xuất logic nguyên nhân lỗi:**
  * Log thành công/bỏ qua được lưu tại: `./log/nenhoang_install_success.log`
  * Log lỗi nặng được lưu tại: `./log/nenhoang_install_error.log`
  * Bản ghi lỗi được cấu trúc hóa giúp SysAdmin biết rõ: Ứng dụng gì, Trạng thái ra sao, Phân tích nguyên nhân cụ thể (Do lỗi chính tả tên gói, hỏng Dependency hay do lỗi mạng 404) đi kèm giải pháp khắc phục trực quan.

## 2. Hướng dẫn thử nghiệm nhanh trên Terminal
```bash
chmod +x bulk_installer.sh
sudo ./bulk_installer.sh apps_list.txt
cat ./log/nenhoang_install_success.log
cat ./log/nenhoang_install_error.log
