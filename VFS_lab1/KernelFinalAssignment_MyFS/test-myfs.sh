#!/bin/bash
echo "[1] Nạp Module Kernel..."
sudo insmod myfs.ko || true

echo "[2] Kiểm tra myfs trong /proc/filesystems..."
grep myfs /proc/filesystems

echo "[3] Tạo điểm mount và mount file system..."
sudo mkdir -p /mnt/myfs
sudo mount -t myfs none /mnt/myfs

echo "[4] Chạy lệnh 'ls -la' trong mount point..."
ls -la /mnt/myfs

echo "[5] Đọc nội dung file ảo bằng 'cat'..."
cat /mnt/myfs/hello.txt

echo ""
echo "[!] Để unmount và xoá module, chạy: sudo umount /mnt/myfs && sudo rmmod myfs"
