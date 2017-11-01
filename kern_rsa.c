/*  KERNEL LEVEL RSA ENCRYTPTION USB DRIVER */


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>


#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x02
#define BULK_EP_IN 0x81
#define MAX_PKT_SIZE 1024

static struct usb_device *device;
static unsigned char bulk_buf[MAX_PKT_SIZE];                      //static buffer for storing data for read and write 


static dev_t first;                                 // Global variable for the first device number

static struct class *cl;                            // Global variable for the device class



// Functions for virtual character interface


struct v_dev                         // Structure which serves as the private data in file pointer
{
 
 unsigned long size;                     /* amount of data stored here */
 void *data;                             //pointer to data area
 struct cdev c_dev;                           // Global variable for the character device structure

};


static struct v_dev dev;



static int v_open(struct inode *i, struct file *f)
{
  
  struct v_dev *dev; 
  printk(KERN_INFO "Driver: open()\n");

  dev = container_of(i->i_cdev, struct v_dev, c_dev);

  if ( (f->f_flags & O_ACCMODE) == O_WRONLY)        //if write only file is truncated
  {
  	dev->size = 0;
  	f->f_pos = 0;
  }
  
  

  f->private_data = dev;


  return 0;
}

static int v_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}
static ssize_t v_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  struct v_dev *dev = f->private_data;

  printk(KERN_INFO "Driver: read() ,len = %d , off = %d ,size = %d \n",len , *off, dev->size);

  if(*off >= dev->size)                 //reached the end of file
  {
  	return 0;
  }

  if (*off + len > dev->size)
  {
  	len = dev->size - *off;
  }

  if (copy_to_user(buf,dev->data + *off,len))
  {
  	return -EFAULT;
  }

  *off += len;                          //updating offset

  return len;
}

static ssize_t v_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
  
  struct v_dev *dev = f->private_data;
  printk(KERN_INFO "Driver: write() ,len = %d , off = %d ,size = %d \n",len , *off, dev->size);

  if(*off + len >= 4096)
  {
  	return 0;
  }

  if (copy_from_user(dev->data + *off, buf, len))
  {
  	return -EFAULT;
  }

  *off += len;                              // updating offset and count;
  if (dev->size < *off)
  	dev->size = *off;


  return len;
}

static struct file_operations v_fops =
{
  .owner = THIS_MODULE,
  .open = v_open,
  .release = v_close,
  .read = v_read,
  .write = v_write
};


//Functions for USB interface with USB subsystem



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
        //printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
        printk(KERN_INFO "");
    }


    // Setting up Virtual Character Interface
 
    
    printk(KERN_INFO "Setting up Virtual Character Interface");
    printk(KERN_INFO "");
  
  	if (alloc_chrdev_region(&first, 0, 1, "kern_rsa_USB") < 0)
   
    {
    	printk(KERN_ERR "Chardev region could not be allocated");
    	printk(KERN_INFO "");
    	return -1;
    }


    if ((cl = class_create(THIS_MODULE, "rsa_usb")) == NULL)
  	{
  		printk(KERN_ERR "Class create failed");
    	printk(KERN_INFO "");
    	unregister_chrdev_region(first, 1);
    	return -1;
  	}

    if (device_create(cl, NULL, first, NULL, "usb0") == NULL)
  	{
  		printk(KERN_ERR "Device create failed");
    	printk(KERN_INFO "");
    	class_destroy(cl);
    	unregister_chrdev_region(first, 1);
    	return -1;
  	}

  	dev.size = 0;
  	dev.data = NULL;
  	dev.data = kmalloc(4096,GFP_KERNEL);
	
  	if (dev.data == NULL)
  	{
  		printk(KERN_ERR "Error. Data memory not available");
  		printk(KERN_INFO "");
	
  		return -1;
	
  	}

    
    cdev_init(&(dev.c_dev), &v_fops);
    if (cdev_add(&(dev.c_dev), first, 1) == -1)
  	{
    	device_destroy(cl, first);
    	class_destroy(cl);
    	unregister_chrdev_region(first, 1);
    	return -1;
  	}


  	printk(KERN_INFO "sys class location : /sys/clas/rsa_usb");
  	printk(KERN_INFO "device location : /dev/usb0");
    printk(KERN_INFO "");


    return 0;
}
 
static void flash_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Pen drive removed\n");
    printk(KERN_INFO "");


  	printk(KERN_INFO "Deregistering Virtual Character Interface");
    printk(KERN_INFO "");

    usb_deregister_dev(interface,&pen_class);

    //deregistering virtual interface

    kfree(dev.data);

    cdev_del(&(dev.c_dev));
    device_destroy(cl, first);
 	class_destroy(cl);
 	unregister_chrdev_region(first, 1);
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
