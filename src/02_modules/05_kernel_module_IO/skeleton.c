#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/ioport.h>
#include <linux/io.h>


static struct resource* region;

//Init function
static int __init skeleton_init(void)
{
	unsigned char* registers;
	
	pr_info ("Linux module 05 skeleton loaded\n");

	//Request memory for chip-ID
	region = request_mem_region (0x01c14200, 0x80, "Zone Chip-Id");
	
	//Memory busy
	if (region == NULL)
		pr_info ("Error request memory\n");

	//Mapping registers for chip-ID
	registers = ioremap (0x01c14200, 0x80);

	//Test if mapping is a success
	if (registers == NULL) 
	{
		pr_info ("Error map register\n");	
	}
	else
	{
		int id[4];

		//Read registers for chip-ID
		id[0] = ioread32 (registers);
		id[1] = ioread32 (registers+0x04);
		id[2] = ioread32 (registers+0x08);
		id[3] = ioread32 (registers+0x0c);

		pr_info("chipid=%08x'%08x'%08x'%08x\n", id[0], id[1], id[2], id[3]);

		//Free registers
		iounmap (registers);
	}

	//Request memory for temperature
	region = request_mem_region (0x01c25080, 0x04, "Zone Temp");
	
	//Memory busy
	if (region == NULL)
		pr_info ("Error request memory\n");

	//Mapping registers for temperature
	registers = ioremap(0x01c25080,0x04);

	//Test if mapping is a success
	if (registers == NULL) 
	{
		pr_info ("Error map register\n");	
	}
	else
	{
		int Temperature = 0;

		//Read register for temperature
		int ValReg = ioread32(registers);

		Temperature = -1191 * ValReg / 10 + 223000;

		pr_info("Temperature : %d\n",Temperature);

		//Free register
		iounmap (registers);
	}

	//Request memory for MAC address
	region = request_mem_region (0x01c30050, 0x08, "Zone MAC");
	
	//Memory busy
	if (region == NULL)
		pr_info ("Error request memory\n");

	//Mapping registers for MAC address
	registers = ioremap(0x01c30050,0x08);

	//Test if mapping is a success
	if (registers == NULL) 
	{
		pr_info ("Error map register\n");	
	}
	else
	{
		//Read registers for MAC address
		int Add1 = ioread32(registers);
		int Add2 = ioread32(registers + 0x04);

		pr_info("Adresse MAC : %02x:%02x:%02x:%02x:%02x:%02x\n",
			(Add2 >> 0) & 0xff,
			(Add2 >> 8) & 0xff,
			(Add2 >> 16) & 0xff,
			(Add2 >> 24) & 0xff,
			(Add1 >> 0) & 0xff,
			(Add1 >> 8) & 0xff);

		//Free registers
		iounmap (registers);
	}

	return 0;
}

//Exit function
static void __exit skeleton_exit(void)
{
	pr_info ("Linux module skeleton unloaded\n");
    if (region != NULL) 
		release_mem_region (0x01c14000, 0x1000);

}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Nathan Bischof <nathan.bischof@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");