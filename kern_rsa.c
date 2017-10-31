/*  KERNEL LEVEL RSA ENCRYTPTION DRIVER */


#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/usb.h>


static int flash_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    printk(KERN_INFO "USB flash drive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    printk(KERN_INFO "");

    return 0;
}
 
static void flash_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Pen drive removed\n");
    printk(KERN_INFO "");
}
 
static struct usb_device_id pen_table[] =
{
    { USB_DEVICE(0x0781, 0x5590) },
    {} 
};

MODULE_DEVICE_TABLE (usb, pen_table);
 
static struct usb_driver pen_driver =
{
    .name = "kern_RSA_driver",
    .id_table = pen_table,
    .probe = flash_probe,
    .disconnect = flash_disconnect,
};
 
static int __init kern_rsa_init(void) 
{
	int result;

    printk(KERN_INFO "kern_rsa Initialized");
    printk(KERN_INFO "");
    result = usb_register(&pen_driver);

    if (result < 0)
    {
    	printk("KERN_INFO" "Error registering device");
    	printk(KERN_INFO "");
    	return -1;
    }

    return 0;
}
 
static void __exit kern_rsa_exit(void) 
{
    printk(KERN_INFO "kern_rsa Uninitialized");
    printk(KERN_INFO "");
    usb_deregister(&pen_driver);
}
 
module_init(kern_rsa_init);
module_exit(kern_rsa_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhiram Haridas");
MODULE_DESCRIPTION("kern_RSA driver");
