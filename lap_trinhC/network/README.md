# Ubuntu Network Interface Management System

Dự án thành phần môn học Kernel Linux (Lập trình quản lý network trong Ubuntu).

## Tính năng
- `list_network_interfaces`: Đọc danh sách card mạng thông qua `getifaddrs` và trích xuất IP bằng `getnameinfo`. Kiểm tra trạng thái UP/DOWN qua ioctl socket.
- `enable_interface` / `disable_interface`: Quản lý trạng thái thiết bị bằng lệnh hệ thống `ip link set`.
- `change_ip_add`: Thay đổi IP tĩnh của một interface. Quá trình bao gồm flush IP cũ và assign IP mới.

## Hướng dẫn sử dụng
1. Mở Terminal tại thư mục này và biên dịch: `make`
2. Chạy chương trình với quyền root (BẮT BUỘC để đổi IP hoặc tắt/bật card mạng): 
   `sudo ./network_manager`

## Xử lý sự cố (Troubleshooting)
- **Lỗi `Lenh that bai voi ma loi 1/2. Ban da chay voi quyen sudo chua?`**: Bạn không có đủ quyền tác động đến Kernel network stack. Hãy khởi chạy lại bằng `sudo`.
- **Mất kết nối SSH/Internet sau khi đổi IP**: Khi bạn can thiệp vào card mạng chính (như `eth0` hoặc `ens33`), cấu hình định tuyến (routing) có thể bị xóa. Hãy cẩn thận khi sử dụng tính năng số (4) trên card mạng đang cung cấp internet. Để thiết lập IP, nên dùng format CIDR (ví dụ: `192.168.1.50/24`).
