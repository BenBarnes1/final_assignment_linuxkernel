#!/bin/bash
echo "[1] Generate a file that we will use as the disk image (dd command)..."
dd if=/dev/zero of=mydisk.img bs=1M count=100

echo "[2] Format the disk with its structure using mkfs.minfs..."
./mkfs.minfs mydisk.img

echo "[3] Load the kernel module (insmod minfs.ko)..."
sudo insmod minfs.ko || true

echo "[4] Check the presence of the minfs file system within /proc/filesystems..."
cat /proc/filesystems | grep minfs

echo "[5] Create mount point /mnt/minfs/ and mount the filesystem..."
sudo mkdir -p /mnt/minfs/
sudo mount -o loop -t minfs mydisk.img /mnt/minfs/

echo "[6] Check that everything is fine by listing the mount point contents /mnt/minfs/..."
ls -l /mnt/minfs/
mount | grep minfs

echo ""
echo "[!] To unmount and clean up, run: sudo umount /mnt/minfs/ && sudo rmmod minfs"
