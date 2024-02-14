# PL2303 uart for linux

hardware : PL2303

distribution : ubuntu for raspberry pi

version : 22.04

baudrate : 9600
## Usage
First, you need to run the makefile.  It will generate the `.ko` `.dtbo` file.
```
make
```
Then, we need to add the device to device tree
```
cp PL2303.dtbo /boot/firmware/overlays/
```
And modify the /boot/firmware/config.txt  Add the following command to config.txt

(In raspberry, uart will conflict bluetooth. We need to close bluetooth that we an use uart)
```
dtoverlay=PL2303
dtoverlay=disable-bt
```
Then, make sure that uart is enable, you can use `raspi-config` to enable it.

Finally, you can insert the module
```
sudo insmod PL2303.ko
```
You can use the following command communicate to hd44780, it will transmit the data
```
echo string > /dev/PL2303_driver
```
