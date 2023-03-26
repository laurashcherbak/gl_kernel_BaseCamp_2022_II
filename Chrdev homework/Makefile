ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := chrdev.o
else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build
default:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif

check:
	../checkpatch.pl --no-tree -f *.c *h

insmod:
	sudo insmod chardev.ko

rmmod:
	sudo rmmod chardev
	
