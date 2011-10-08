KERNELDIR=/usr/src/linux-2.6
MAKE=make
PWD=$(shell pwd)

obj-m := qqmodule.o
module-objs := qqmodule.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
