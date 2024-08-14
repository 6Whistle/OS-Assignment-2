obj-m := ftracehooking.o iotracehooking.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CFLAG := -DEXPORT_SYMTAB

all:
	$(MAKE) $(CLFAG) -C $(KDIR) SUBDIRS=$(PWD) modules
cleen:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
