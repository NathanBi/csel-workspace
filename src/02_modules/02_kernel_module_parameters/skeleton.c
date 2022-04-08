#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/moduleparam.h>

//Parameters
static char* text = "Defaut";
module_param(text, charp, 0);
static int  elements = 0;
module_param(elements, int, 0);

//Init function
static int __init skeleton_init(void)
{
    pr_info ("Linux module skeleton loaded\n");

    //Print parameters
    pr_info ("Para 1 (text) : %s\n",text);
    pr_info ("Para 2 (elements) : %d\n",elements);
   
    return 0;
}

//Exit function
static void __exit skeleton_exit(void)
{
    pr_info ("Linux module skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");