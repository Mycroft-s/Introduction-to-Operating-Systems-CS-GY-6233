obj-m += lab7.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all: lab7_user lab7.ko

lab7_user: lab7_user.c
	$(CC) -Wall -O2 -o lab7_user lab7_user.c

lab7.ko:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f lab7_user
