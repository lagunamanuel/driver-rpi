MODULE = driver_rpi
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

obj-m += $(MODULE).o

.PHONY: compile install uninstall clean

compile:
	$(MAKE) -C "$(KERNEL_SRC)" M="$(CURDIR)" modules

install:
	sudo insmod $(MODULE).ko

uninstall:
	sudo rmmod $(MODULE)

clean:
	$(MAKE) -C "$(KERNEL_SRC)" M="$(CURDIR)" clean
