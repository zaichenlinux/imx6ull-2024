
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

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_CNT			1		  	
#define NEWCHRLED_NAME			"newchrled"	
#define LEDOFF 					0			
#define LEDON 					1			

// phy addr
#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

// virtual addr
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct newchrled_dev {
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	int major;
	int minor;
};

struct newchrled_dev newchrled;

void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON) {
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO1_DR);
	}else if(sta == LEDOFF) {
		val = readl(GPIO1_DR);
		val|= (1 << 3);	
		writel(val, GPIO1_DR);
	}	
}

static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &newchrled;
	printk("led open ==\r\n");
	return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	printk("led read ===\r\n");
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* ��ȡ״ֵ̬ */

	if(ledstat == LEDON) {	
		led_switch(LEDON);		/* ��LED�� */
	} else if(ledstat == LEDOFF) {
		led_switch(LEDOFF);	/* �ر�LED�� */
	}
	printk("led write ====\r\n");
	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	printk("led release =====\r\n");
	return 0;
}

static struct file_operations newchrled_fops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.read  = led_read,
	.write = led_write,
	.release = led_release,
};

static int __init led_init(void)
{
	u32 val = 0;

	/* ��ʼ��LED */
	/* 1���Ĵ�����ַӳ�� */
  	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
  	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	/* 2��ʹ��GPIO1ʱ�� */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* �����ǰ������ */
	val |= (3 << 26);	/* ������ֵ */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3������GPIO1_IO03�ĸ��ù��ܣ����临��Ϊ
	 *    GPIO1_IO03���������IO���ԡ�
	 */
	writel(5, SW_MUX_GPIO1_IO03);
	
	/*�Ĵ���SW_PAD_GPIO1_IO03����IO����
	 *bit 16:0 HYS�ر�
	 *bit [15:14]: 00 Ĭ������
     *bit [13]: 0 kepper����
     *bit [12]: 1 pull/keeperʹ��
     *bit [11]: 0 �رտ�·���
     *bit [7:6]: 10 �ٶ�100Mhz
     *bit [5:3]: 110 R0/6��������
     *bit [0]: 0 ��ת����
	 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4������GPIO1_IO03Ϊ������� */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* �����ǰ������ */
	val |= (1 << 3);	/* ����Ϊ��� */
	writel(val, GPIO1_GDIR);

	/* 5��Ĭ�Ϲر�LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);

	// register chrdev
	if(newchrled.major){
		newchrled.devid = MKDEV(newchrled.major,0);
		register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
	}else {
		alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);
		newchrled.major = MAJOR(newchrled.devid);
		newchrled.minor = MINOR(newchrled.devid);
	}
	printk("newchrled major=%d, minor=%d\r\n",newchrled.major, newchrled.minor);

	// init chrdev
	newchrled.cdev.owner  = THIS_MODULE;
	cdev_init(&newchrled.cdev, &newchrled_fops);

	// add cdev
	cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);

	// create class
	newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
	if(IS_ERR(newchrled.class)){
		return PTR_ERR(newchrled.class);
	}

	// create device
	newchrled.device = device_create(newchrled.class,NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
	if(IS_ERR(newchrled.device)){
		return PTR_ERR(newchrled.device);
	}	
	printk("led init!\r\n");
	return 0;
}

static void __exit led_exit(void)
{
	/* ȡ��ӳ�� */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* ע���ַ��豸���� */
	cdev_del(&newchrled.cdev);/*  ɾ��cdev */
	unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT); /* ע���豸�� */

	device_destroy(newchrled.class, newchrled.devid);
	class_destroy(newchrled.class);
	printk("led exit!\r\n");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MR_CHEN");

