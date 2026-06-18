# Ubuntu File Management System

Dự án thành phần môn học Kernel Linux (Lập trình quản lý tiến trình, file, socket trong Ubuntu).

## Tính năng
- `create_file`: Tạo mới một tệp tin (sử dụng `fopen`).
- `display_file`: Đọc và in nội dung tệp tin an toàn (tránh tràn bộ đệm bằng `fgets`).
- `edit_file`: Tích hợp với trình soạn thảo `nano` thông qua `system()` call.
- `check_permissions`: Kiểm tra cờ phân quyền tệp (sử dụng `sys/stat.h`).
- `delete_file`: Xóa tệp khỏi hệ thống tệp.
- `compress_file` / `decompress_file`: Gọi system tool `gzip` để nén/giải nén tệp.

## Hướng dẫn biên dịch và sử dụng
1. Mở terminal tại thư mục này.
2. Biên dịch mã nguồn: `make`
3. Chạy chương trình: `./file_manager`

## Xử lý sự cố (Troubleshooting)
- Lỗi `Khong the khoi tao tien trinh chinh sua file`: Đảm bảo `nano` đã được cài đặt trên hệ thống (`sudo apt install nano`).
- Lỗi phân quyền khi đọc/xóa: Kiểm tra lại với tính năng số (4) hoặc sử dụng `sudo ./file_manager` nếu file thuộc sở hữu của root.
- Lệnh nén/giải nén thất bại: Đảm bảo công cụ `gzip` khả dụng.
