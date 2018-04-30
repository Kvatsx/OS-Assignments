all:
	dd status=noxfer conv=notrunc if=boot2.bin of=boot2.flp
	qemu-system-i386 -fda boot2.flp

compile:
	nasm -f bin -o boot2.bin boot2.asm
	dd status=noxfer conv=notrunc if=boot2.bin of=boot2.flp
	qemu-system-i386 -fda boot2.flp
	
iso:
	mkdir cdiso
	mv boot2.flp /cdiso
	mkisofs -o boot2.iso -b boot2.flp cdiso/