obj-m += indigo.o 
indigo-objs += indigo_utils.o main.o math.o libgraph_hack.o
ccflags-y := -mhard-float -msse -g -O0

all: 
	cp libgraph.pic.a libgraph_hack.o_shipped
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
 
