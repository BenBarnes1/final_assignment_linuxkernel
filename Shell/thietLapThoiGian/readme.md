# 2. Hướng dẫn khởi chạy

## Bước 1: Cấp quyền thực thi cho file script

`chmod +x time_manager.sh`
```
### Bước 2: Xem thời gian hiện hành (Không cần quyền root)

./time_manager.sh view

### Bước 3: Thay đổi giờ hệ thống (Bắt buộc chạy với sudo)

`sudo ./time_manager.sh chhour`


### Bước 4: Thay đổi ngày hệ thống (Bắt buộc chạy với sudo)


sudo ./time_manager.sh chdate


* *Kịch bản thử nghiệm:* Nhập một ngày không tồn tại (Ví dụ: `20260231`), bộ lọc kiểm tra biên sẽ báo lỗi: `Ngày nhập vào không tồn tại trong thực tế: Ngày 31 tháng 02 năm 2026.`


