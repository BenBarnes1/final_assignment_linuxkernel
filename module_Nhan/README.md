# Mini-Firewall (Netfilter Kernel Module)

Dự án thành phần môn học Kernel Linux (Xây dựng mô-đun nhân tương tác Network Stack).
Mô-đun này sử dụng kiến trúc `Netfilter` để đánh chặn ở mức `sk_buff` và tự động loại bỏ (Drop) tất cả các yêu cầu Ping (ICMP) đến máy.

## Hướng dẫn trải nghiệm độ "Ngầu" của mô-đun

1. **Biên dịch mô-đun:**
   ```bash
   make
Thử nghiệm trước khi nạp (Mở 1 terminal khác):
Hãy thử ping chính máy của bạn hoặc để máy khác trong LAN ping tới máy bạn:

Bash
ping 127.0.0.1
(Bạn sẽ thấy ping trả về kết quả bình thường)

Nạp mô-đun để kích hoạt Tường lửa:

Bash
sudo insmod mini_firewall.ko
(Ngay lập tức, lệnh ping ở terminal kia sẽ bị đứng khựng lại, không thể nhận phản hồi)

Xem log Kernel bắt quả tang gói tin:

Bash
dmesg | tail -n 5
(Bạn sẽ thấy dòng log: [MINI-FIREWALL] Phat hien goi tin ICMP... Da huy (DROP)!)

Gỡ bỏ mô-đun để phục hồi trạng thái:

Bash
sudo rmmod mini_firewall
make clean
(Terminal đang ping sẽ ngay lập tức chạy tiếp)
