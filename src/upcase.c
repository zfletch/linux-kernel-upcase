#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zachary Fletcher <zfletch2@gmail.com>");
MODULE_DESCRIPTION("In-kernel text upper-caser");

static int __init upcase_init(void);
static void __exit upcase_exit(void);

static unsigned long int buffer_size = 8192;

module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");
module_init(upcase_init);
module_exit(upcase_exit);

static int __init upcase_init(void)
{
	if (buffer_size == 0) {
		return -1;
	}

	printk(KERN_INFO "upcase device registered with buffer size %lu bytes\n", buffer_size);
	return 0;
}

static void __exit upcase_exit(void)
{
	printk(KERN_INFO "upcase device unregistered\n");
}


