#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define	GPIOLED_CNT		1
#define	GPIOLED_NAME	"gpioled"
#define	LEDOFF			0
#define	LEDON			1

struct gpioled_dev{
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	int major;
	int minor;
	struct device_node *nd;
	int led_gpio;
};

struct gpioled_dev gpioled;

static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &gpioled;
	printk("led open!\r\n");
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	struct gpioled_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* ��ȡ״ֵ̬ */

	if(ledstat == LEDON) {	
		gpio_set_value(dev->led_gpio, 0);	/* ��LED�� */
	} else if(ledstat == LEDOFF) {
		gpio_set_value(dev->led_gpio, 1);	/* �ر�LED�� */
	}
	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations gpioled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

static int __init led_init(void)
{
	int ret = 0;

	/* ����LED��ʹ�õ�GPIO */
	/* 1����ȡ�豸�ڵ㣺gpioled */
	gpioled.nd = of_find_node_by_path("/gpioled");
	if(gpioled.nd == NULL) {
		printk("gpioled node not find!\r\n");
		return -EINVAL;
	} else {
		printk("gpioled node find!\r\n");
	}

	/* 2�� ��ȡ�豸���е�gpio���ԣ��õ�LED��ʹ�õ�LED��� */
	gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
	if(gpioled.led_gpio < 0) {
		printk("can't get led-gpio");
		return -EINVAL;
	}
	printk("led-gpio num = %d\r\n", gpioled.led_gpio);

	/* 3������GPIO1_IO03Ϊ�������������ߵ�ƽ��Ĭ�Ϲر�LED�� */
	ret = gpio_direction_output(gpioled.led_gpio, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* ע���ַ��豸���� */
	/* 1�������豸�� */
	if (gpioled.major) {		/*  �������豸�� */
		gpioled.devid = MKDEV(gpioled.major, 0);
		register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
	} else {						/* û�ж����豸�� */
		alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);	/* �����豸�� */
		gpioled.major = MAJOR(gpioled.devid);	/* ��ȡ����ŵ����豸�� */
		gpioled.minor = MINOR(gpioled.devid);	/* ��ȡ����ŵĴ��豸�� */
	}
	printk("gpioled major=%d,minor=%d\r\n",gpioled.major, gpioled.minor);	
	
	/* 2����ʼ��cdev */
	gpioled.cdev.owner = THIS_MODULE;
	cdev_init(&gpioled.cdev, &gpioled_fops);
	
	/* 3�����һ��cdev */
	cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

	/* 4�������� */
	gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(gpioled.class)) {
		return PTR_ERR(gpioled.class);
	}

	/* 5�������豸 */
	gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
	if (IS_ERR(gpioled.device)) {
		return PTR_ERR(gpioled.device);
	}
	return 0;
}

static void __exit led_exit(void)
{
	/* ע���ַ��豸���� */
	cdev_del(&gpioled.cdev);/*  ɾ��cdev */
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* ע���豸�� */

	device_destroy(gpioled.class, gpioled.devid);
	class_destroy(gpioled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MR_CHEN");


