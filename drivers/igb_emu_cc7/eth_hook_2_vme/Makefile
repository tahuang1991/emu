ifneq ($(KERNELRELEASE),)
obj-m	:= eth_hook_2_vme.o

else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
KBUILD_EXTRA_SYMBOLS := /usr/src/kernels/3.10.0-693.5.2.el7.x86_64/Module.symvers

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -fr .tmp_versions *.o *.ko *.ko.unsigned *.mod *.mod.c .*.cmd Module.markers Module.symvers modules.order

endif
