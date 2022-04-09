#include <linux/module.h> /* needed by all modules */
#include <linux/init.h> // needed for macros
#include <linux/kernel.h> // needed for debugging
#include <linux/device.h> /* needed for sysfs handling */
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#define CHAR_BUFFER_SIZE 1000

struct skeleton_data {
    int id;
    char name[30];
};

static struct skeleton_data data;

// Char device
static struct cdev driver_cdev;

static dev_t driver_dev;

ssize_t sysfs_show_data(struct device* dev, struct device_attribute* attr, char* buf)
{
    sprintf(buf,"%d %s\n",data.id, data.name);

    return strlen(buf);
}

ssize_t sysfs_store_data(struct device* dev, struct device_attribute* attr,const char* buf,size_t count)
{
    sscanf(buf,"%d %s\n", &data.id, data.name);
    return count;
}

DEVICE_ATTR(Data, 0664, sysfs_show_data, sysfs_store_data);

// static struct class* sysfs_class;
static struct device* sysfs_device;

static int driver_open(struct inode* i, struct file* f)
{
    printk("driver_open: major %d, minor %d\n", imajor(i), iminor(i));
    // Opening with appending intent
    if ((f->f_flags & (O_APPEND)) != 0)
    {
        printk("driver : opened for appending data...\n");
    }
    // Opening with reading and writing intent
    else if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0) 
    {
        printk("driver : opened for reading & writing...\n");
    } 
    // Opening with reading intent
    else if ((f->f_mode & FMODE_READ) != 0) 
    {
        printk("driver : opened for reading...\n");
    } 
    // Opening wiht writing intent
    else if ((f->f_mode & FMODE_WRITE) != 0) 
    {
        printk("driver : opened for writing...\n");
    }
    return 0;
}

// Reading contents of device file
static ssize_t driver_read(struct file* f, char* __user buf, size_t count, loff_t* off)
{
    // Remaining space in buffer left after write operation
    ssize_t remaining = CHAR_BUFFER_SIZE - (ssize_t)(*off);
    // Apply offset
    char* buf_ptr = (char*)f->private_data + *off;

    int cpy_status = 0;
    // Is enough space available for storing further bytes
    if (count >= remaining)
    {
        count = remaining;
    }
    // Compute new offset
    *off += count;
    // Copy from storage buffer to output buffer
    cpy_status = copy_to_user(buf, buf_ptr, count);
    if (cpy_status != 0)
    {
        count = -EFAULT;
    } 

    printk("driver_read\n");
    return count;
}

// Writing contents into device file
static ssize_t driver_write(struct file* f, const char* __user buf, size_t count, loff_t* off)
{
    int cpy_status = 0;

    // Remaining space in buffer left after write operation
    ssize_t remaining = CHAR_BUFFER_SIZE - (ssize_t)(*off);
    printk("driver_write function\n");

    // Is enough space available for storing further bytes
    if (count >= remaining)
    {
        count = -EIO;
    }

    // Save/store further entered bytes in the buffer
    if (count > 0)
    {
        char* buf_ptr = (char*)f->private_data + *off;
        // Compute new offset
        *off += count;
        buf_ptr[count] = 0; // null terminated
        // Copy from input buffer to storage buffer
        cpy_status = copy_from_user(buf_ptr, buf, count);
        if(cpy_status)
        {
            count = -EFAULT;
        }
    }
    return count;
}

// Closing of device file
static int driver_release(struct inode *i, struct file *f)
{
    printk("driver_release \n");
    return 0;
}

static struct file_operations sysfs_fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .read = driver_read,
    .write = driver_write,
    .release = driver_release,
};

static struct miscdevice misc_sysfs_device = {
    .minor = MISC_DYNAMIC_MINOR, // dynamic minor attribution
    .name = "misc_sysfs_device",
    .mode = 0, // rights under /dev dir
    .fops = &sysfs_fops, // returns to regular file_operations struct
};



static int __init skeleton_init(void)
{
    // Dynamic allocation of driver numbers to avoid conflicts with other devices
    alloc_chrdev_region(&driver_dev, 0, 1, "driver_module");

    // Char device initialization
    cdev_init(&driver_cdev, &sysfs_fops);

    // Save driver to kernel
    cdev_add(&driver_cdev, driver_dev, 1);

    // Create sysfs file
    misc_register(&misc_sysfs_device);
    device_create_file(sysfs_device, &dev_attr_Data);

    pr_info("Linux driver module sysfs loaded\n");
    return 0;
}

static void __exit skeleton_exit(void)
{
    device_remove_file(sysfs_device, &dev_attr_Data);
    misc_deregister(&misc_sysfs_device);
    cdev_del(&driver_cdev);
    unregister_chrdev_region(driver_dev, 1);
    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");