obj-m += indigo.o 
indigo-objs += indigo_utils.o main.o math.o libgraph.pic.a
ccflags-y := -mhard-float -msse -g -O0

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
 
