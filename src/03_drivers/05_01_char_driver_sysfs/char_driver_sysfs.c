#include <linux/module.h> /* needed by all modules */
#include <linux/init.h> // needed for macros
#include <linux/kernel.h> // needed for debugging
#include <linux/device.h> /* needed for sysfs handling */
#include <linux/platform_device.h>

struct skeleton_data {
    int id;
    char name[30];
};

static struct skeleton_data data;

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
// static struct device* sysfs_device;

static struct platform_device sysfs_device = {
    .name       = "sysfs_module",
    .id         = -1,
    .dev.release= sysfs_dev_release,
};

static int __init skeleton_init(void)
{
    int status = 0;

    if(status == 0)
    {
        status = platform_device_register(&sysfs_device);
    }
    if(status == 0)
    {
        status = device_create_file(&sysfs_device.dev, &dev_attr_Data);
    }

    // sysfs_class = class_create(THIS_MODULE, "my_sysfs_class");
    // sysfs_device = device_create(sysfs_class, NULL, 0, NULL, "my_sysfs_device");

    // device_create_file(sysfs_device, &dev_attr_Data);

    pr_info("Linux module skeleton loaded\n");
    return status;
}

static void __exit skeleton_exit(void)
{

    device_remove_file(&sysfs_device.dev, &dev_attr_Data);
    // device_destroy(sysfs_class, 0);
    // class_destroy(sysfs_class);
    platform_device_unregister(&sysfs_device);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");