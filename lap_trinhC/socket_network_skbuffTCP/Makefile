obj-m += stego_module.o
KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules
	gcc chat_bi.c -o chat_bi -pthread
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f chat_bi
