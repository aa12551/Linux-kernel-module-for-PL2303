#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/cdev.h>

static int serdev_echo_probe(struct serdev_device *serdev);
static void serdev_echo_remove(struct serdev_device *serdev);
struct serdev_device* serdev_p = NULL;

static dev_t dev;
static struct class *uart_class;
static struct cdev *uart_cdev;


static struct of_device_id serdev_echo_ids[] = {
	{
		.compatible = "PL2303",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, serdev_echo_ids);

static struct serdev_device_driver serdev_echo_driver = {
	.probe = serdev_echo_probe,
	.remove = serdev_echo_remove,
	.driver = {
		.name = "PL2303",
		.of_match_table = serdev_echo_ids,
	},
};

static int serdev_echo_recv(struct serdev_device *serdev, 
                            const unsigned char *buffer, size_t size) 
{
	printk("serdev_echo - Received %ld bytes with %s\n", size, buffer);
        return 	serdev_device_write_buf(serdev, buffer, size);
}


static const struct serdev_device_ops serdev_echo_ops = {
	.receive_buf = serdev_echo_recv,
};

static int serdev_echo_probe(struct serdev_device *serdev) 
{	
        /* Create serial device bus and set the uart */
    
	if(serdev_device_driver_register(&serdev_echo_driver)) {
		printk("serdev_echo - Error! Could not load driver\n");
		return -1;
	}

	serdev_device_set_client_ops(serdev, &serdev_echo_ops);
	if(serdev_device_open(serdev)) {
		printk("serdev_echo - Error opening serial port!\n");
		goto serdev_error;
	}

	serdev_device_set_baudrate(serdev, 9600);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);
        serdev_p = serdev;

        serdev_device_write_buf(serdev, "Type something: " \
        , sizeof("Type something: "));
        
        return 0;

serdev_error:
        serdev_device_driver_unregister(&serdev_echo_driver);   
        return 0;
}

static void serdev_echo_remove(struct serdev_device *serdev) 
{
	serdev_device_driver_unregister(&serdev_echo_driver);
        serdev_device_close(serdev);
}

static ssize_t serdev_write(struct file *file, const char __user *buf, 
                            size_t len, loff_t *ppos) 
{
        int copied, no_copied, write_bytes;
        char *data = kmalloc(len, GFP_KERNEL);
        no_copied = copy_from_user(data, buf, len);
        copied = len - no_copied;
        write_bytes = serdev_device_write_buf(serdev_p, data, copied);

        return len - no_copied;
}


static const struct file_operations uart_fops = {
        .owner = THIS_MODULE,
        .write = serdev_write
};

static int __init my_init(void) {

        /* Create the device class and add the character device to the class */

        if (alloc_chrdev_region(&dev, 0, 1, "PL2303")) {
                printk("Failed to register character device\n");
                return -1;
        }
    
        if((uart_class = class_create(THIS_MODULE, "PL2303_class")) == NULL) {
                printk("Failed to create device class\n");
                goto class_error;
        }

        if(device_create(uart_class, NULL, dev, NULL, "PL2303_driver") == NULL) {
                printk("Failed to create device\n");
                goto device_error;
        }   

        if((uart_cdev = cdev_alloc()) == NULL) {
                printk("Failed to allocate charcter device\n");
                goto alloc_error;
        }

        cdev_init(uart_cdev, &uart_fops);

        if(cdev_add(uart_cdev, dev, 1)) {
                printk("Failed to add the character device");
                goto add_error;
        }   
	return 0;

add_error:
        cdev_del(uart_cdev);
alloc_error:
        device_destroy(uart_class, dev);
device_error:
        class_destroy(uart_class);
class_error:
        unregister_chrdev_region(dev, 1);
    
        return 0; 
}

static void __exit my_exit(void) {
        cdev_del(uart_cdev);
        device_destroy(uart_class, dev);
        class_destroy(uart_class);
        unregister_chrdev_region(dev, 1);
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("David");
MODULE_DESCRIPTION("UART driver about PL2303");

module_init(my_init);
module_exit(my_exit);
