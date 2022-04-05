#include <linux/module.h> //needed by all modules
#include <linux/init.h>   //needed for macros
#include <linux/kernel.h> //needed for debugging
#include <linux/kthread.h> //needed for threads
#include <linux/delay.h> //needed for ssleep function
#include <linux/interrupt.h> //needed for interrupt
#include <linux/gpio.h> //needed for io
// #include <linux/wait.h> // needed wait_queues


// Interrupt service routine
irqreturn_t irq_handler(int irq, void *dev_id)
{
    printk("Interrupt raised, button %s seems to have been pressed.\n", (char*) dev_id);

    return IRQ_HANDLED;
}

// Execution on insmod
static int __init kernel_module_init(void)
{
    int gpio_status = 0;
    int irq_status = 0;

    // Button K1
    gpio_status = gpio_request(0, "K1");
    irq_status = request_irq(gpio_to_irq(0), irq_handler, IRQF_TRIGGER_FALLING, "K1", "K1");
    // Button K2
    gpio_status = gpio_request(2, "K2");
    irq_status = request_irq(gpio_to_irq(2), irq_handler, IRQF_TRIGGER_FALLING, "K2", "K2");
    // Button K3
    gpio_status = gpio_request(3, "K3");
    irq_status = request_irq(gpio_to_irq(3), irq_handler, IRQF_TRIGGER_FALLING, "K3", "K3");

    pr_info("Linux interrupt kernel module loaded, with gpio return code %d and irq return code %d.\n", gpio_status, irq_status);

    return 0;
}

// Execution on rmmod
static void __exit kernel_module_exit(void)
{
    // Button K1
    gpio_free(0);
    free_irq(gpio_to_irq(0), "K1");
    // Button K2
    gpio_free(2);
    free_irq(gpio_to_irq(2), "K2");
    // Button K3
    gpio_free(3);
    free_irq(gpio_to_irq(3), "K3");

    pr_info ("Linux interrupt kernel module unloaded.\n");
}

module_init (kernel_module_init);
module_exit (kernel_module_exit);

MODULE_AUTHOR ("Guillaume Blin");
MODULE_DESCRIPTION ("Module Thread");
MODULE_LICENSE ("GPL");