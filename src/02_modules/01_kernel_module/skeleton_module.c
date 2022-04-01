#include <linux/module.h> //needed by all modules
#include <linux/init.h>   //needed for macros
#include <linux/kernel.h> //needed for debugging



static int __init skeleton_init(void)
{
    pr_info ("Linux skeleton module loaded.\n");
    return 0;
}

static void __exit skeleton_exit(void)
{
    pr_info ("Linux skeleton module unloaded.\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Guillaume Blin");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");