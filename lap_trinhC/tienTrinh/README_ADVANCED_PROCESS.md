# Tài liệu Module: Quản lý Tiến trình Chuyên sâu (process_master.c)

Hệ thống điều khiển và giám sát tiến trình viết bằng ngôn ngữ C, can thiệp trực tiếp vào kiến trúc kernel space ảo `/proc` và điều hướng phân tách luồng POSIX Process trên hệ điều hành Ubuntu.

## 1. Các giải pháp công nghệ nâng cao đã triển khai

* **Giám sát chuyên sâu thông qua cấu trúc `/proc/[PID]/status`:** Thay vì chỉ trích xuất tên cơ bản, ứng dụng thực hiện quét và bóc tách chuỗi nhị phân để tìm các trường thuộc tính `State` (Trạng thái thực tế: S (Sleeping), R (Running), Z (Zombie)) và `VmSize` (Dung lượng bộ nhớ ảo tiến trình đang chiếm dụng).
* **Ủy quyền thực thi toàn diện (`fork` + `execvp`):** Tiến trình con được sinh ra không chạy chung code với cha, mà sử dụng hàm `execvp` để thay thế hoàn toàn không gian địa lý bộ nhớ của nó bằng một chương trình Linux độc lập. Cơ chế bẻ chuỗi bằng `strtok` đảm bảo các tham số truyền vào (Arguments) được định dạng đúng chuẩn mảng con trỏ kết thúc bằng `NULL`.
* **Vòng lặp phi nghẽn (Non-blocking Wait với WNOHANG):** Để vừa cho phép cha theo dõi con, vừa sẵn sàng nhận tín hiệu ngắt từ người dùng (Ctrl+C), hàm `waitpid` được cấu hình với cờ hiệu `WNOHANG`. Kết hợp với hàm `usleep(100000)` (100ms) giúp hạ tải mức độ quét của CPU xuống mức 0%, khắc phục hoàn toàn lỗi busy-waiting.
* **Xử lý Tín hiệu Tùy biến:** Tích hợp menu ánh xạ hệ thống tín hiệu POSIX để SysAdmin linh hoạt xử lý sự cố (`SIGSTOP` để đóng băng tài nguyên nghi vấn, `SIGCONT` để khôi phục hoặc `SIGKILL` để giải phóng bộ nhớ lập tức).

## 2. Hướng dẫn vận hành

### Bước 1: Biên dịch mã nguồn tối ưu bằng GCC
```bash
gcc -Wall -Wextra -o process_master process_master.c
Bước 2: Khởi chạy ứng dụng bằng quyền thường hoặc quyền root
Bash
./process_master
Kịch bản thử nghiệm tính năng số 3 (Execvp):
Chọn tùy chọn 3.

Nhập một lệnh hệ thống có thời gian thực thi (Ví dụ: ping -c 5 google.com).

Tiến trình con sẽ in kết quả ping ra màn hình. Trong lúc nó đang chạy, bạn hãy nhấn tổ hợp phím Ctrl + C.

Kết quả đạt được: Tiến trình cha ngay lập tức bắt được tín hiệu ngắt, gửi lệnh SIGKILL triệt tiêu tiến trình ping ngầm, gọi waitpid dọn dẹp bộ nhớ và đưa bạn quay về Menu an toàn, không làm crash ứng dụng chính.
