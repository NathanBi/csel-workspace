#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>

//Parameters
static char* text = "Defaut";
module_param(text, charp, 0);
static int  elements = 10;
module_param(elements, int, 0);

//Element from the list
struct element {
	char titre[10];
	int nb;
	struct list_head node;
};

//List
static LIST_HEAD (my_list);

//Init function
static int __init skeleton_init(void)
{
	pr_info ("Linux module 04 skeleton loaded\n");

	//Print parameters
	pr_info ("Para 1 (text) : %s\n",text);
    pr_info ("Para 2 (elements) : %d\n",elements);

	//Create elements with kmalloc
	for (int i = 0; i < elements; i++) {
		struct element* ele; 
		ele = kmalloc (sizeof(*ele), GFP_KERNEL);
		if (ele != NULL)
		{
			//Copy parameters to element's data
			strncpy(ele->titre,text,sizeof(ele->titre));
			ele->nb = i;

			//Add element to the list
			list_add_tail (&ele->node, &my_list);
		}
	}
	return 0;
}

//Exit function
static void __exit skeleton_exit(void)
{
	struct element* ele;

	//Until the list is empty
	while (!list_empty (&my_list)) 
	{
		//Select the next element in the list
		ele = list_entry (my_list.next, struct element, node);
		pr_info ("Element N %d : %s\n", ele->nb, ele->titre);

		//Delete the element from the list
		list_del (&ele->node);

		//Free ele
		kfree (ele);
	}
	
	pr_info ("Linux module skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");