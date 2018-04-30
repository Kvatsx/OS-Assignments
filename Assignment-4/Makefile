obj-m += enc_dev.o
	
obj-m += dec_device.o

all: 
	make -C /lib/modules/$(uname -r)/build M=$PWD modules

mknod:
	mknod -m 666 /dev/encdev c 220 0
	mknod -m 666 /dev/decdev c 230 0

install:
	insmod ./dec_device.ko
	insmod ./enc_dev.ko
	gcc test.c -o test

clean:
	rmmod enc_dev
	rmmod dec_device
	rm enc_dev.ko
	rm dec_device.ko
	rm test