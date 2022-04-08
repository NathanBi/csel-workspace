export PATH := /buildroot/output/host/usr/sbin$\
	:/buildroot/output/host/usr/bin/$\
	:/buildroot/output/host/sbin$\
	:/buildroot/output/host/bin/$\
	:$(PATH)

# Part executed when called from kernel build system
ifneq ($(KERNELRELEASE),)
obj-m += char_driver_sysfs.o ## name of the generated module

ymodule-objs := char_driver_sysfs.o ## list of objets needed by that module
CFLAGS_skeleton := -DDEBUG ## to enable printing of debugging messages

# Part executed when called from standard make in the module source directory
else
include ../../../kernel_settings

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
endif