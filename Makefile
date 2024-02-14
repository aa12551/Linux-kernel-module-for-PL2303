obj-m += PL2303.o

all: module dt

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: PL2303.dts
	dtc -@ -I dts -O dtb -o PL2303.dtbo PL2303.dts
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf PL2303.dtbo

