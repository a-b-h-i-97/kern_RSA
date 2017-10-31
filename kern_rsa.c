/*  KERNEL LEVEL RSA ENCRYTPTION DRIVER */


#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
 
static int __init kern_rsa_init(void) 
{
    printk(KERN_INFO "kern_rsa Initialized");
    return 0;
}
 
static void __exit kern_rsa_exit(void) 
{
    printk(KERN_INFO "kern_rsa Uninitialized");
}
 
module_init(kern_rsa_init);
module_exit(kern_rsa_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhiram Haridas");
MODULE_DESCRIPTION("kern_RSA driver");
