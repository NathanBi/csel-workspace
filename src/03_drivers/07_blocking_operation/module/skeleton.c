#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>

static char* btnk1 = "Bouton K1";
static char* btnk2 = "Bouton K2";
static char* btnk3 = "Bouton K3";

//Waitqueue and notify var
static int request_can_be_processed;
DECLARE_WAIT_QUEUE_HEAD(queue);

//Operations for the file
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .poll  = skeleton_poll,
};

//Device parameters
struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops  = &fops,
    .name  = "mymodule",
    .mode  = 0777,
};

//Interrupt function
irqreturn_t gpio_isr(int irq, void* handle)
{

    //Notify and wake up thread
    request_can_be_processed = 1;
    wake_up_interruptible(&queue);

    pr_info("Interruption de %s\n", (char*)handle);

    return IRQ_HANDLED;
}

//Poll function
static unsigned int skeleton_poll(struct file* f, poll_table* wait)
{
    unsigned mask = 0;

    //Wait
    poll_wait(f, &queue, wait);

    //Test if we have an interrupt
    if (request_can_be_processed != 0) {
        mask |= POLLIN | POLLRDNORM; /* read operation */
        request_can_be_processed = 0;
        pr_info("Thread Waked-Up\n");
    }
    return mask;
}

//Init function
static int __init skeleton_init(void)
{
    int status = 0;
    request_can_be_processed = 0;

    //create the device
    misc_register(&misc_device);

    //Define the interrupt for K1
   if (status == 0)  
        status = devm_request_irq(misc_device.this_device,
                                  gpio_to_irq(0),
                                  gpio_isr,
                                  IRQF_TRIGGER_FALLING | IRQF_SHARED,
                                  btnk1,
                                  btnk1);

    //Define the interrupt for K2
    if (status == 0)
        status = devm_request_irq(misc_device.this_device,
                                  gpio_to_irq(2),
                                  gpio_isr,
                                  IRQF_TRIGGER_RISING | IRQF_SHARED,
                                  btnk2,
                                  btnk2);

    //Define the interrupt for K3
    if (status == 0)
        status = devm_request_irq(misc_device.this_device,
                                  gpio_to_irq(3),
                                  gpio_isr,
                                  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_SHARED,
                                  btnk3,
                                  btnk3);

    pr_info("Linux module skeleton loaded(status=%d)\n", status);
    return status;
}

//Exit function
static void __exit skeleton_exit(void)
{
    //Delete the device
    misc_deregister(&misc_device);

    pr_info("Linux module skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");