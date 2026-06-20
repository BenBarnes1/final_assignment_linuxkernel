obj-m += minfs.o
KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc mkfs.minfs.c -o mkfs.minfs

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f mkfs.minfs mydisk.img
