#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David");
MODULE_DESCRIPTION("Simple program");
MODULE_VERSION("0.01");

static int __init kernel_init(void) {
    printk(KERN_INFO "Hello, Check Point!\n");
    return 0;
}

static void __exit kernel_exit(void) {
    printk(KERN_INFO "Goodbye, Check Point!\n");
}

module_init(kernel_init);
module_exit(kernel_exit);
