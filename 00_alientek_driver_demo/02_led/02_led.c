#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define	LED_MAJAR	200
#define	LED_NAME	"led"

#define	LED_OFF	0
#define	LED_ON	1

// PHY ADDR
#define	CCM_CCGR1_BASE				( 0X020C406C )
#define	SW_MUX_GPIO1_IO03_BASE		( 0X020E0068 )
#define	SW_PAD_GPIO1_IO03_BASE		( 0X020E02F4 )
#define	GPIO1_DR_BASE				( 0X0209C000 )
#define	GPIO1_GDIR_BASE				( 0X0209C004 )

// Virtual ADDR
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

void led_switch(uint8_t sta)
{
	uint32_t val = 0;
	if(sta == LED_ON){
		val = readl(GPIO1_DR);
		val &=~(1 << 3);
		writel(val, GPIO1_DR);
	}else if{
		val = readl(GPIO1_DR);
		val = val | (1 <<3);
		writel(val,GPIO1_DR);
	}

}

static int led_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt )
{
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0){
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];

	if(ledstat == LED_ON){
		led_switch(LED_ON);
	}else if(ledstat == LED_OFF){
		led_switch(LED_OFF);
	}
	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	printk("led release!\r\n");
	return 0;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.read  = led_read,
	.write = led_write,
	.release = led_release,
};

static int __init led_init(void)
{
	int retvalue = 0;
	uint32_t val = 0;

	// reg addr map
	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	// enable GPIO1 clk
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);
	val |= (3 << 26);
	writel(val, IMX6U_CCM_CCGR1);

	// 复用为GPIO1_IO03
	writel(5, SW_MUX_GPIO1_IO03);

	// GPIO1_IO03的属性
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	// Set output 
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);
	val |= (1 << 3);
	writel(val, GPIO1_GDIR);

	// default close led
	val = readl(GPIO1_DR);
	val |= (1 << 3);
	writel(val, GPIO1_DR);

	retvalue = register_chrdev(LED_MAJAR, LED_NAME, &led_fops);
	if(retvalue < 0){
		printk("register chrdev failed!\r\n");
		return -EIO;
	}

	return 0;
}

static void __exit led_exit(void)
{
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	unregister_chrdev(LED_MAJAR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MR_CHEN");

