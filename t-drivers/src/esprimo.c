#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>	// copy_{to,from}_user

#define DEVICE_NAME "esprimo"


// Estructura propia del device
struct esprimo_dev {
    struct cdev cdev;
};
struct esprimo_dev esprimo_dev;

// Device number
static dev_t esprimo_devno;

// Class
static struct class *esprimo_class;

// file_operations que se asociara al cdev
ssize_t esprimo_read(struct file *filp, char __user *data, size_t s, loff_t *off);
ssize_t esprimo_write(struct file *filp, const char __user *data, size_t s, loff_t *off);
static struct file_operations esprimo_fops = {
    .owner = THIS_MODULE,
    .read = esprimo_read,
    .write = esprimo_write,
};

// numero que vamos a ver si es primo o no. Lo inicializamos en 2
static int number = 2;

static int __init esprimo_init(void) {
    printk(KERN_ALERT "esprimo: load\n");
    // Pedimos un device number al kernel dinamicamente
    if(alloc_chrdev_region(&esprimo_devno, 0, 1, DEVICE_NAME)) {
        printk(KERN_DEBUG "esprimo: No se pudo registrar el device\n");
        return 1;
    }

    // Conectamos el file_operations con el cdev
    cdev_init(&esprimo_dev.cdev, &esprimo_fops);
    esprimo_dev.cdev.owner = THIS_MODULE;

    // Conectamos el device number al cdev
    if (cdev_add(&esprimo_dev.cdev, esprimo_devno, 1)) {
        printk(KERN_DEBUG "esprimo: Error al agregar el char device\n");
        return 1;
    }

    // Hacemos que se creen los nodos en /dev
    esprimo_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(esprimo_class, NULL, esprimo_devno, NULL,
        DEVICE_NAME);
	return 0;
}

static void __exit esprimo_exit(void) {

    printk(KERN_ALERT "esprimo: unload\n");
    // Quitamos el cdev
    cdev_del(&esprimo_dev.cdev);

    // Liberamos el major number
    unregister_chrdev_region(MAJOR(esprimo_devno), 1);

    // Destruimos la clase y el device
    device_destroy(esprimo_class, esprimo_devno);
    class_destroy(esprimo_class);
}

static int is_prime(int num)
{
	int i;
	//printk(KERN_ALERT "esprimo: %d es primo ?\n", num);
	for (i = 2; i < num; i++) {
		if (num % i == 0)
			return 0;
	}

	return 1;
}

ssize_t esprimo_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
	ssize_t a;
	char ret_str;
	//printk(KERN_ALERT "Han leido al esprimo tamaño %d\n", s);
	printk(KERN_ALERT "esprimo: se ha leido: %d\n", number);

	if (is_prime(number)) {
		printk(KERN_ALERT "esprimo: %d es primo!\n", number);
		ret_str = '1';
	} else {
		printk(KERN_ALERT "esprimo: %d NO es primo!\n", number);
		ret_str = '0';
	}

	a = copy_to_user(data, &ret_str, s);
	if (a != 0)
		return s - a;

	return s;
}

ssize_t esprimo_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char str[count];
	int new_number = 0;
	ssize_t a;
	ssize_t i;
	//printk(KERN_ALERT "Han escrito esprimo con size: %d!\n", count);

	a = copy_from_user(str, buf, count);
	if (a != 0)
		return count - a;

	//printk(KERN_ALERT "valor: %s!\n", str);
	for (i = 0; i < count; i++) {
		char c = str[i];
		if (c < '0' || c > '9')
			continue;
		//printk(KERN_ALERT "convirtiendo: %c!\n", c);
		new_number *= 10;
		new_number += (c - '0');
		//printk(KERN_ALERT "new_number por ahora es: %d!\n", new_number);
	}
	number = new_number;
	printk(KERN_ALERT "esprimo: han escrito el numero: %d\n", number);


	return count;
}


module_init(esprimo_init);
module_exit(esprimo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rodrigo Campos");
MODULE_DESCRIPTION("cardev esprimo");
