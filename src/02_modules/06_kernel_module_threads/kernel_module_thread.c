#include <linux/module.h> //needed by all modules
#include <linux/init.h>   //needed for macros
#include <linux/kernel.h> //needed for debugging
#include <linux/kthread.h> //needed for threads
#include <linux/delay.h> //needed for ssleep function

// Thread declared globally
static struct task_struct* thread;

// Thread function
static int kernel_module_thread(void *arg)
{
    int message_id = 0;
    printk(KERN_INFO "Thread: %s[PID = %d]\n", current->comm, current->pid);
    printk("Going into infinite loop...\n");
    
    // runs until kthread_stop function is called
    while(!kthread_should_stop())
    {
        message_id = message_id + 1;
        printk("This is a new message with id %d \n", message_id);
        ssleep(5);
    } 
    return 0;
}

// Execution on insmod
static int __init kernel_module_init(void)
{
    int err;
    thread = kthread_run(kernel_module_thread, 0, "Kernel Module Thread");
    
    if(IS_ERR(thread))
    {
        printk("Error in creation or run of thread\n");
        err = PTR_ERR(thread);
        thread = NULL;
        return err;
    }
    pr_info ("Linux thread kernel module loaded.\n");
    return 0;
}

// Execution on rmmod
static void __exit kernel_module_exit(void)
{
    kthread_stop(thread);
    pr_info ("Linux thread kernel module unloaded.\n");
}

module_init (kernel_module_init);
module_exit (kernel_module_exit);

MODULE_AUTHOR ("Guillaume Blin");
MODULE_DESCRIPTION ("Module Thread");
MODULE_LICENSE ("GPL");