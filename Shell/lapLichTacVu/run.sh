# 1. Đảm bảo file có quyền chạy
chmod +x cron_manager.sh backup_task.sh run.sh

# 2. Tạo sẵn thư mục source và file test (Bắt buộc phải có dữ liệu trong source)
mkdir -p source backup log
echo "Test dữ liệu" > source/test.txt

# 3. Lấy đường dẫn tuyệt đối của file backup để tí nữa paste vào code
pwd
# Lệnh pwd sẽ in ra đường dẫn, ví dụ: /home/nenhoang/Final_Assignment/Shell/lapLichTacVu
# Đường dẫn đầy đủ của file backup sẽ là: /home/nenhoang/Final_Assignment/Shell/lapLichTacVu/backup_task.sh

# 4. Tiến hành lập lịch chạy mỗi phút (*) để test ngay lập tức
./cron_manager.sh create