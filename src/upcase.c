#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zachary Fletcher <zfletch2@gmail.com>");
MODULE_DESCRIPTION("In-kernel text upper-caser");

struct upcase_buffer {
	char* data;
	char* end;
	char* location;
	size_t size;
	wait_queue_head_t read_queue;
};

static int __init upcase_init(void);
static void __exit upcase_exit(void);
static struct upcase_buffer* upcase_buffer_alloc(size_t size);
static void upcase_buffer_free(struct upcase_buffer* buffer);
static int upcase_open(struct inode* inode, struct file* file);
static int upcase_close(struct inode* inode, struct file* file);
static ssize_t upcase_read(struct file* file, char* __user out, size_t size, loff_t* off);
static ssize_t upcase_write(struct file* file, const char* __user in, size_t size, loff_t* off);
static inline void upcase_string(char* start, char* end);

static struct file_operations upcase_fops = {
	.owner = THIS_MODULE,
	.open = upcase_open,
	.read = upcase_read,
	.write = upcase_write,
	.release = upcase_close,
	.llseek = noop_llseek,
};

static struct miscdevice upcase_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "upcase",
	.fops = &upcase_fops,
};

static unsigned long int buffer_size = 8192;

module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");
module_init(upcase_init);
module_exit(upcase_exit);

static struct upcase_buffer* upcase_buffer_alloc(size_t size)
{
	struct upcase_buffer* buffer;
	buffer = kzalloc(sizeof(struct upcase_buffer), GFP_KERNEL);
	if (unlikely(buffer == NULL)) {
		goto out_error;
	} else {
		buffer->data = kzalloc(size, GFP_KERNEL);

		if (unlikely(buffer->data == NULL)) {
			kfree(buffer);
			goto out_error;
		}

		buffer->size = size;
		buffer->location = buffer->data;
		buffer->end = buffer->data;
		init_waitqueue_head(&buffer->read_queue);
		goto out_success;
	}

out_success:
	return buffer;

out_error:
	return NULL;
}

static void upcase_buffer_free(struct upcase_buffer* buffer)
{
	if (buffer != NULL) {
		kfree(buffer->data);
		kfree(buffer);
	}
}

static int upcase_open(struct inode *inode, struct file *file)
{
	struct upcase_buffer* buffer = upcase_buffer_alloc(buffer_size);

	if (unlikely(buffer == NULL)) {
		goto out_error;
	} else {
		file->private_data = (void*) buffer;
		goto out_success;
	}

out_success:
	return 0;

out_error:
	return -ENOMEM;
}

static int upcase_close(struct inode *inode, struct file *file)
{
	upcase_buffer_free((struct upcase_buffer*) file->private_data);

	return 0;
}

static ssize_t upcase_read(struct file* file, char* __user out, size_t size, loff_t* off)
{
	struct upcase_buffer* buffer = (struct upcase_buffer*) file->private_data;
	ssize_t result;

	while (buffer->location == buffer->end) {
		if (file->f_flags & O_NONBLOCK) {
			result = -EAGAIN;
			goto out;
		}
		if (wait_event_interruptible(buffer->read_queue, buffer->location != buffer->end)) {
			result = -ERESTARTSYS;
			goto out;
		}
	}

	size = min(size, (size_t) (buffer->end - buffer->location));
	if (copy_to_user(out, buffer->location, size)) {
		result = -EFAULT;
		goto out;
	}

	buffer->location += size;
	result = size;
	goto out;

out:
	return result;

}

static ssize_t upcase_write(struct file* file, const char* __user in, size_t size, loff_t* off)
{
	struct upcase_buffer* buffer = (struct upcase_buffer*) file->private_data;
	ssize_t result;

	if (size > buffer_size) {
		result = -EFBIG;
		goto out;
	}
	if (copy_from_user(buffer->data, in, size)) {
		result = -EFAULT;
		goto out;
	}

	buffer->end = buffer->data + size;
	buffer->location = buffer->data;

	if (buffer->end > buffer->data) {
		upcase_string(buffer->data, buffer->end);
	}

	wake_up_interruptible(&buffer->read_queue);
	result = size;
	goto out;

out:
	return result;

}

static inline void upcase_string(char* start, char* end)
{
	while (start < end) {
		if (*start <= 'z' && *start >= 'a') {
			*start += 'A' - 'a';
		}
		start += 1;
	}
}

static int __init upcase_init(void)
{
	if (buffer_size == 0) {
		return -1;
	}

	misc_register(&upcase_device);

	printk(KERN_INFO "upcase device registered with buffer size %lu bytes\n", buffer_size);
	return 0;
}

static void __exit upcase_exit(void)
{
	misc_deregister(&upcase_device);
	printk(KERN_INFO "upcase device unregistered\n");
}


