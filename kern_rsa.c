/*  KERNEL LEVEL RSA ENCRYTPTION USB DRIVER */


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>


#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x02
#define BULK_EP_IN 0x81
#define MAX_PKT_SIZE 1024

static struct usb_device *device;
static unsigned char bulk_buf[MAX_PKT_SIZE];                      //static buffer for storing data for read and write 




static int pen_open(struct inode *i, struct file *f)              //returning 0 for open and close
{
    return 0;
}
static int pen_close(struct inode *i, struct file *f)
{
    return 0;
}
static ssize_t pen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int read_cnt;
 
    /* Read the data from the bulk endpoint */
    retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN),
            bulk_buf, MAX_PKT_SIZE, &read_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Error. Bulk message read returned %d\n", retval);
        return retval;
    }
    if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
    {
        return -EFAULT;
    }
 
    return MIN(cnt, read_cnt);
}
static ssize_t pen_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);
 
    if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }
 
    /* Write the data into the bulk endpoint */
    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT),
            bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Error. Bulk message write returned %d\n", retval);
        return retval;
    }
 
    return wrote_cnt;
}
 
static struct file_operations fops =
{
    .open = pen_open,
    .release = pen_close,
    .read = pen_read,
    .write = pen_write,
};

static struct usb_class_driver pen_class = {

	.name = "pen%d",
	.fops = &fops,
};


static int flash_probe(struct usb_interface *interface, const struct usb_device_id *id)
{

	int retval;
 
    device = interface_to_usbdev(interface);

    printk(KERN_INFO "USB flash drive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    printk(KERN_INFO "");    
 

    if ((retval = usb_register_dev(interface, &pen_class)) < 0)                  //registering character interface for device
    {
        printk(KERN_ERR "Error. Minor not obtained");
        printk(KERN_INFO "");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
        printk(KERN_INFO "");
    }
 
    return retval;

    return 0;
}
 
static void flash_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Pen drive removed\n");
    printk(KERN_INFO "");

    usb_deregister_dev(interface,&pen_class);
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
