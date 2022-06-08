#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/thermal.h>
#include <linux/gpio.h>
#include <linux/timer.h>

#define LED 10

//Structure for the Data file
struct skeleton_data {
    int mode;
    int frequency;
};

static struct skeleton_data data;
struct thermal_zone_device * zone;

static struct timer_list my_timer;

void my_timer_callback(struct timer_list *timer)
{
    int temp = 0;
    int duty = 0;
    static int led = 0;

    if(data.mode == 1)
    {
        thermal_zone_get_temp(zone,&temp);

        temp /= 1000;

        if(temp < 35)
        {
            duty = 500;
        }
        else if(temp < 40)
        {
            duty = 200;
        }
        else if(temp < 45)
        {
            duty = 100;
        }
        else if (temp >= 45)
        {
            duty = 50;
        }
    }
    else
    {
        duty = data.frequency;
    }

    gpio_set_value(LED,led);

    led = !led;

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(duty));
}


//Show function
ssize_t sysfs_show_data(struct device* dev, struct device_attribute* attr, char* buf)
{
    sprintf(buf,"%d %d\n",data.mode, data.frequency);

    return strlen(buf);
}

//Store function
ssize_t sysfs_store_data(struct device* dev, struct device_attribute* attr,const char* buf,size_t count)
{
    int mode_tmp, frequency_tmp;

    sscanf(buf,"%d %d\n", &mode_tmp, &frequency_tmp);

    if(mode_tmp == 0 || mode_tmp == 1)
    {
        data.frequency = frequency_tmp;
        data.mode = mode_tmp;
    }
    else
        printk("Erreur : mode incorrect");

    return count;
}

//Assign stor ans show function
DEVICE_ATTR(Data, 0664, sysfs_show_data, sysfs_store_data);

static struct class* sysfs_class;
static struct device* sysfs_device;

//Init function
static int __init skeleton_init(void)
{
    data.mode = 1;
    data.frequency = 2;

    zone = thermal_zone_get_zone_by_name("cpu-thermal");

    //Create sysfs class
    sysfs_class  = class_create(THIS_MODULE, "my_sysfs_class");
    sysfs_device = device_create(sysfs_class, NULL, 0, NULL, "my_sysfs_device");

    device_create_file(sysfs_device, &dev_attr_Data);

    gpio_request(LED,"gpioa.10-led");
    gpio_direction_output(LED,0);

    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(100));

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

    gpio_set_value(LED,0);
    gpio_free(LED);

    del_timer(&my_timer);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");
