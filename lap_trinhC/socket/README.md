# Ubuntu Socket Programming (Chat Server/Client)

Dự án thành phần môn học Kernel Linux (Lập trình socket và network trong Ubuntu).

## Cấu trúc kỹ thuật
- Sử dụng mô hình `TCP Socket` (AF_INET, SOCK_STREAM).
- Quản lý đa tiến trình bằng `pthread` (Multithreading).
- Đồng bộ hóa tài nguyên chung trên Server bằng `pthread_mutex` tránh Race Conditions.
- Khắc phục lỗi Socket Time-Wait bằng cờ `SO_REUSEADDR`.

## Hướng dẫn sử dụng
1. Mở Terminal tại thư mục này và biên dịch: `make`
2. Khởi chạy Server (ở một Terminal): `./server`
3. Khởi chạy Client (ở một/nhiều Terminal khác): `./client`

## Xử lý sự cố (Troubleshooting)
- **Lỗi `Address already in use`**: Server đóng đột ngột làm port bị treo ở trạng thái TIME_WAIT. `SO_REUSEADDR` đã được cấu hình trong mã để ngăn chặn điều này. Nếu vẫn bị, kiểm tra bằng lệnh `lsof -i :50000` và `kill` PID tương ứng.
- **Lỗi `Connection refused`**: Bạn đang cố chạy `./client` khi `./server` chưa được bật, hoặc địa chỉ IP/Port trong `B2.h` không khớp.
