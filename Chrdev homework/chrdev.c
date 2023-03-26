// SPDX-License-Identifier: GPL-3.0-or-later

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Laura Shcherbak");
MODULE_DESCRIPTION("Character device driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

#define CLASS_NAME "chrdev"
#define DEVICE_NAME "chrdev_example"
#define DIR_NAME "chrdev_example_dir"
#define BUFFER_SIZE 1024
#define PROC_BUFFER_SIZE (BUFFER_SIZE + 100)

static struct class *pclass;
static struct device *pdev;
static struct cdev chrdev_cdev;
dev_t dev;

static int major;
static int is_open;
static bool is_read;

static int data_size;
static unsigned char data_buffer[BUFFER_SIZE];

static char proc_buffer[PROC_BUFFER_SIZE];

static struct proc_dir_entry *proc_file;
static struct proc_dir_entry *proc_folder;

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (is_open) {
		pr_err("chrdev: already open\n");
		return -EBUSY;
	}
	is_open = 1;
	pr_info("chrdev: device opened");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	is_open = 0;
	pr_info("chrdev: device closed\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int ret;

	pr_info("chrdev: read from file %s\n", filep->f_path.dentry->d_iname);
	pr_info("chrdev: read from device %d:%d\n", imajor(filep->f_inode), iminor(filep->f_inode));

	if (len > data_size)
		len = data_size;

	ret = copy_to_user(buffer, data_buffer, len);
	if (ret) {
		pr_err("chrdev: copy_to_user failed: %d\n", ret);
		return -EFAULT;
	}
	data_size = 0;

	pr_info("chrdev: %zu bytes read\n", len);
	return len;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int ret;

	pr_info("chrdev: write to file %s\n", filep->f_path.dentry->d_iname);
	pr_info("chrdev: write to device %d:%d\n", imajor(filep->f_inode), iminor(filep->f_inode));

	data_size = len;
	if (data_size > BUFFER_SIZE)
		data_size = BUFFER_SIZE;

	ret = copy_from_user(data_buffer, buffer, data_size);
	if (ret) {
		pr_err("chrdev: copy_from_user failed: %d\n", ret);
		return -EFAULT;
	}

	pr_info("chrdev: %d bytes written\n", data_size);
	return data_size;
}

static struct file_operations fops = {
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write,
};

static ssize_t proc_read(struct file *FIle, char __user *buffer, size_t Count, loff_t *offset)
{
	size_t size1;
	size_t size2;

	is_read = !is_read;
	if (!is_read)
		return 0;

	size1 = sprintf(proc_buffer, "Size of buffer: %d.\nCapasity of buffer: %d.\nBuffer contains:\n%s\n", data_size, BUFFER_SIZE, data_buffer);

	if (size1 > Count)
		size1 = Count;

	size2 = copy_to_user(buffer, proc_buffer, size1);

	return size1 - size2;
}

//static ssize_t proc_write(struct file *FIle, const char __user *buffer, size_t count, loff_t *offset) //$$$
//{
//	return count;
//}

static const struct proc_ops proc_fops = {
	.proc_read = proc_read
	//.proc_write = proc_write //$$
};

static int chrdev_init(void)
{
	is_open = 0;
	data_size = 0;

	major = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);

	if (major < 0) {
		pr_err("chrdev: register_chrdev failed: %d\n", major);
		return major;
	}
	pr_info("chrdev: register_chrdev ok, major = %d minor %d\n", MAJOR(dev), MINOR(dev));

	cdev_init(&chrdev_cdev, &fops);
	if ((cdev_add(&chrdev_cdev, dev, 1)) < 0) {
		pr_err("chrdev: cannot add the device to the system\n");
		goto cdev_err;
	}
	pr_info("chrdev: cdev created successfully\n");

	pclass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(pclass))
		goto class_err;
	pr_info("chrdev: device class created successfully\n");

	pdev = device_create(pclass, NULL, dev, NULL, CLASS_NAME"0");
	if (IS_ERR(pclass))
		goto device_err;
	pr_info("chrdev: device node created successfully\n");

	//proc
	proc_folder = proc_mkdir(DIR_NAME, NULL);
	if (!proc_folder) {
		pr_err("chrdev: create /proc/%s/ folder failed.\n", DIR_NAME);
		goto device_err;
	}
	pr_info("chrdev: proc folder /proc/%s/ created successfully.\n", DIR_NAME);

	proc_file = proc_create(DEVICE_NAME, 0444, proc_folder, &proc_fops);
	if (!proc_file) {
		pr_err("chrdev: initialize /proc/%s/%s failed.\n", DIR_NAME, DEVICE_NAME);
		goto proc_err;
	}
	pr_info("chrdev: /proc/%s/%s initialized successfully.\n", DIR_NAME, DEVICE_NAME);

	pr_info("chrdev: module init successfully.\n");

	return 0;

proc_err: //proc
	proc_remove(proc_file);
	proc_remove(proc_folder);
device_err:
	class_destroy(pclass);
class_err:
	cdev_del(&chrdev_cdev);
cdev_err:
	unregister_chrdev_region(dev, 1);
	return -1;
}

static void chrdev_exit(void)
{
	//proc
	proc_remove(proc_file);
	proc_remove(proc_folder);

	device_destroy(pclass, dev);
	class_destroy(pclass);
	cdev_del(&chrdev_cdev);
	unregister_chrdev_region(dev, 1);

	pr_info("chrdev: module exited\n");
}

module_init(chrdev_init);
module_exit(chrdev_exit);
