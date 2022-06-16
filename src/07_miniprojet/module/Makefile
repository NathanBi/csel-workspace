export PATH := /buildroot/output/host/usr/sbin$\
	:/buildroot/output/host/usr/bin/$\
	:/buildroot/output/host/sbin$\
	:/buildroot/output/host/bin/$\
	:$(PATH)

# Part executed when called from kernel build system
ifneq ($(KERNELRELEASE),)
obj-m += thermal_module.o ## name of the generated module
mymodule-objs := thermal_module.o ## list of objets needed by that module
CFLAGS_skeleton := -DDEBUG ## to enable printing of debugging messages
MODPATH  := /rootfs
# Part executed when called from standard make in the module source directory
else
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.21/
TOOLS := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
PWD := $(shell pwd)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
endif