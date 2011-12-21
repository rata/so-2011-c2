#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "esqueleto"

ssize_t esqueleto_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
    printk(KERN_ALERT "Han leido al esqueleto!\n");
    return 0;
}

// Estructura propia del device
struct esqueleto_dev {
    struct cdev cdev;
};
struct esqueleto_dev esqueleto_dev;

// file_operations que se asociara al cdev
static struct file_operations esqueleto_fops = {
    .owner = THIS_MODULE,
    .read = esqueleto_read,
};

// Device number
static dev_t esqueleto_devno;

// Class
static struct class *esqueleto_class;


static int __init esqueleto_init(void) {
    // Pedimos un device number al kernel dinamicamente
    if(alloc_chrdev_region(&esqueleto_devno, 0, 1, DEVICE_NAME)) {
        printk(KERN_DEBUG "esqueleto: No se pudo registrar el device\n");
        return 1;
    }

    // Conectamos el file_operations con el cdev
    cdev_init(&esqueleto_dev.cdev, &esqueleto_fops);
    esqueleto_dev.cdev.owner = THIS_MODULE;

    // Conectamos el device number al cdev
    if (cdev_add(&esqueleto_dev.cdev, esqueleto_devno, 1)) {
        printk(KERN_DEBUG "esqueleto: Error al agregar el char device\n");
        return 1;
    }

    // Hacemos que se creen los nodos en /dev
    esqueleto_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(esqueleto_class, NULL, esqueleto_devno, NULL,
        DEVICE_NAME);
	return 0;
}

static void __exit esqueleto_exit(void) {
    // Quitamos el cdev
    cdev_del(&esqueleto_dev.cdev);

    // Liberamos el major number
    unregister_chrdev_region(MAJOR(esqueleto_devno), 1);

    // Destruimos la clase y el device
    device_destroy(esqueleto_class, esqueleto_devno);
    class_destroy(esqueleto_class);
}

module_init(esqueleto_init);
module_exit(esqueleto_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pablo Antonio");
MODULE_DESCRIPTION("esqueleto de chardev");
