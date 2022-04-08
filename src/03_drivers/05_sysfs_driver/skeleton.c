#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>

//Structure for the Data file
struct skeleton_data {
    int id;
    char name[30];
};

static struct skeleton_data data;

//Show function
ssize_t sysfs_show_data(struct device* dev, struct device_attribute* attr, char* buf)
{
    sprintf(buf,"%d %s\n",data.id, data.name);

    return strlen(buf);
}

//Store function
ssize_t sysfs_store_data(struct device* dev, struct device_attribute* attr,const char* buf,size_t count)
{
    sscanf(buf,"%d %s\n", &data.id, data.name);
    return count;
}

//Assign stor ans show function
DEVICE_ATTR(Data, 0664, sysfs_show_data, sysfs_store_data);

static struct class* sysfs_class;
static struct device* sysfs_device;

//Init function
static int __init skeleton_init(void)
{
    int status = 0;

    //Create sysfs class
    sysfs_class  = class_create(THIS_MODULE, "my_sysfs_class");
    sysfs_device = device_create(sysfs_class, NULL, 0, NULL, "my_sysfs_device");

    device_create_file(sysfs_device, &dev_attr_Data);

    pr_info("Linux module skeleton loaded\n");
    return 0;
}

//Exit function
static void __exit skeleton_exit(void)
{
    //Remove sysfs class
    device_remove_file(sysfs_device, &dev_attr_Data);
    device_destroy(sysfs_class, 0);
    class_destroy(sysfs_class);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");
