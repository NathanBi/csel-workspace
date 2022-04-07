#include <linux/module.h> //needed by all modules
#include <linux/init.h>   //needed for macros
#include <linux/kernel.h> //needed for debugging
#include <linux/kthread.h> //needed for threads
#include <linux/delay.h> //needed for ssleep function
#include <linux/interrupt.h> //needed for interrupt
#include <linux/gpio.h> //needed for io
#include <linux/fs.h>
#include <linux/cdev.h> // Needed for cdev struct
#include <linux/uaccess.h> //needed for copy from/to user

// #define OWNER "Guillaume Blin"

#define CHAR_BUFFER_SIZE 1000

// Module parameter example: insmod kernel_driver.ko 
static int amount_instances;
module_param(amount_instances, int, 0);

static char* char_buffer[CHAR_BUFFER_SIZE];

// Char device
static struct cdev driver_cdev;

static dev_t driver_dev;


// Opening of device file
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

// Closing of device file
static int driver_release(struct inode *i, struct file *f)
{
    printk("driver_release \n");
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

// File Operations
static struct file_operations driver_fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .read = driver_read,
    .write = driver_write,
    .release = driver_release,
};

// Execution on insmod
static int __init kernel_module_init(void)
{
    int i = 0;
    // Dynamic allocation of driver numbers to avoid conflicts with other devices
    alloc_chrdev_region(&driver_dev, 0, amount_instances, "driver_module");

    // Char device initialization
    cdev_init(&driver_cdev, &driver_fops);

    // Save driver to kernel
    cdev_add(&driver_cdev, driver_dev, amount_instances);

    for(i = 0; i < amount_instances; i++)
    {
        char_buffer[i] = kzalloc(sizeof(char*) * CHAR_BUFFER_SIZE, GFP_KERNEL);
        printk("Instance %d allocated", i);
    }
    printk("Linux char driver kernel module loaded.\n");

    return 0;
}

// Execution on rmmod
static void __exit kernel_module_exit(void)
{
    int i = 0;
    // Removing of driver from kernel
    cdev_del(&driver_cdev);
    // Free driver numbers
    unregister_chrdev_region(driver_dev, 1);
    for(i = 0; i < amount_instances; i++)
    {
        kfree(char_buffer[i]);
    }

    printk("Linux char driver kernel module unloaded.\n");
}

module_init (kernel_module_init);
module_exit (kernel_module_exit);

MODULE_AUTHOR ("Guillaume Blin");
MODULE_DESCRIPTION ("Module Thread");
MODULE_LICENSE ("GPL");