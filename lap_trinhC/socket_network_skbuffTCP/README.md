# TCP Urgent Pointer Steganography

## Cách vận hành
1. **Biên dịch:** Chạy lệnh `make`
2. **Nạp mô-đun Kernel:** `sudo insmod stego_module.ko`
3. **Mở luồng theo dõi Kernel:** Mở một terminal riêng và chạy `dmesg -w` để theo dõi tin nhắn được bóc tách.
4. **Chạy ứng dụng Chat:**
   - Tại máy 1: `sudo ./chat_bi server`
   - Tại máy 2 (hoặc terminal khác): `sudo ./chat_bi client 127.0.0.1`
5. Nhập tin nhắn và xem điều kỳ diệu bên cửa sổ `dmesg`.
