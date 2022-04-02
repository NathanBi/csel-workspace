#include <linux/module.h> //needed by all modules
#include <linux/init.h>   //needed for macros
#include <linux/kernel.h> //needed for debugging
#include <linux/kthread.h> //needed for threads
#include <linux/delay.h> //needed for ssleep function
// #include <linux/wait.h> // needed wait_queues

// Thread declared globally
static struct task_struct* detect_thread;
static struct task_struct* notif_thread;

// Atomic
static atomic_t is_kicked;

// Static initialization of a wait_queue
DECLARE_WAIT_QUEUE_HEAD(wait_queue);

// THREAD: Notification detector thread
static int kernel_module_notification_detector_thread(void *arg)
{
    printk(KERN_INFO "Thread: %s[PID = %d]\n", current->comm, current->pid);

    // runs until kthread_stop function is called
    while(!kthread_should_stop())
    {
        int wait_event_status = wait_event_interruptible (wait_queue, (kthread_should_stop() || atomic_read(&is_kicked) != 0));
        if (wait_event_status == -ERESTARTSYS)
        {
            printk("Thread [PID = %d] was interrupted (-ERESTARTSYS)\n", current->pid);
            break;
        }
        else
        {
            printk("Thread [PID = %d] is kicked by another thread\n", current->pid);
            atomic_set(&is_kicked, 0);
        } 
    }

    return 0;
}

// THREAD: Notification raiser thread
static int kernel_module_notification_raiser_thread(void *arg)
{
    // Dynamic wait queue
    wait_queue_head_t dynamic_wait_queue;
    init_waitqueue_head(&dynamic_wait_queue);

    printk(KERN_INFO "Thread: %s[PID = %d]\n", current->comm, current->pid);
    
    // runs until kthread_stop function is called
    while(!kthread_should_stop())
    {
        // printk("Thread kicks other thread\n");

        // Raise notification to other thread
        atomic_set(&is_kicked, 1);
        wake_up_interruptible(&wait_queue);
        ssleep(5);
    } 
    return 0;
}

// Execution on insmod
static int __init kernel_module_init(void)
{
    // Error management
    int err_detect_thread;
    int err_notif_thread;

    // detect thread start
    detect_thread = kthread_run(kernel_module_notification_detector_thread, 0, "Kernel Module Notification Detector Thread");
    if(IS_ERR(detect_thread))
    {
        printk("Error in creation or run of thread\n");
        err_detect_thread = PTR_ERR(detect_thread);
        detect_thread = NULL;
        return err_detect_thread;
    }

    // notif thread start
    notif_thread = kthread_run(kernel_module_notification_raiser_thread, 0, "Kernel Module Notification Thread");
    if(IS_ERR(notif_thread))
    {
        printk("Error in creation or run of thread\n");
        err_notif_thread = PTR_ERR(notif_thread);
        notif_thread = NULL;
        return err_notif_thread;
    }

    // Init atomic
    atomic_set(&is_kicked, 0);

    pr_info ("Linux thread kernel module loaded.\n");
    return 0;
}

// Execution on rmmod
static void __exit kernel_module_exit(void)
{
    kthread_stop(detect_thread);
    kthread_stop(notif_thread);
    pr_info ("Linux thread kernel module unloaded.\n");
}

module_init (kernel_module_init);
module_exit (kernel_module_exit);

MODULE_AUTHOR ("Guillaume Blin");
MODULE_DESCRIPTION ("Module Thread");
MODULE_LICENSE ("GPL");