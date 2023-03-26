# Home work

● Chrdev homework: part 1

Write simple character device driver.

Buffer size ~1k,

Handlers: read(), write(), open(), release()

Proc: read() chrdev buffer print

# Bash

● Prepare sources
```
git clone --depth 1 --branch v5.3 git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git

make clean
make mrproper
export BUILD_KERNEL=/path/to/kernel/sources
git checkout v5.3 -b work_5_3
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- multi_v7_defconfig
make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- zImage
```

● Fix multiple definition of 'yylloc' error
```vi scripts/dtc/dtc-lexer.lex.c
```

● checkpatch.pl
```checkpatch.pl --no-tree -f chrdev.c
```

Build for ARM architecture
```
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- KDIR=../BaseCamp/linux
```

● Working with module in Linux
```shell
sudo insmod chrdev.ko
sudo lsmod | grep chrdev
sudo rmmod chrdev.ko
```

# Results

dmesg

tail -f /var/log/kern.log
