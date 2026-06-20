obj-m += myfs.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	@echo "[BUILD] Bien dich myfs file system..."
	make -C $(KDIR) M=$(PWD) modules

clean:
	@echo "[CLEAN] Xoa cac file tam..."
	make -C $(KDIR) M=$(PWD) clean
