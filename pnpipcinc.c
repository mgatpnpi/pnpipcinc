/* pnpipcinc: Petersburg Nuclear Physics Institute PCI Neutron Counter device driver */
/* 
 * Author: Mikhail Golubev
 * Organization: LO, Gatchina, Orlova Roscha, NRC KI PNPI
 * hardware: Altera Cyclon v2 + plx9030
 *  2018
 *
 *
 * driver developed for Reflectometer of Polarized Neutrons scientific setup
 *
 * This exotic scientific hardware driver is licensed under General Public License
 *
 *
 *
 * */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "pnpipcinc.h"



static struct pci_device_id pnpipcinc_ids[] = {
	{ PCI_DEVICE( 0x10b5, 0x90f1) },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, pnpipcinc_ids);


struct pnpi_device {
	int number; // 0 or 1
	char type[8]; // timer or counter
	dev_t majorminor;
	struct cdev cdev;
	struct class* pnpi_device_class;
	unsigned long cs0_port;
	unsigned long cs1_port;
	void __iomem *cs2_mem_addr;
	void __iomem *cs3_mem_addr;
};

struct pnpi_pci_data_t {
	struct pnpi_device counter0;
	struct pnpi_device counter1;
	struct pnpi_device timer0;
	struct pnpi_device timer1;
};


static int pnpi_open(struct inode *inode, struct file * f)
{
	struct pnpi_device *pnpi_dev;
	pnpi_dev = container_of(inode->i_cdev, struct pnpi_device, cdev);
	f->private_data = pnpi_dev;

	return 0;
}

static long timer_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
	struct pnpi_device *pnpi_dev = f->private_data;
	int number = pnpi_dev->number;

	switch (ioctl_num)
	{
		case IOCTL_CMD_REG_FREQUENCY:
			iowrite8(ioctl_param, pnpi_dev->cs2_mem_addr+CS2_ADDR_OFFSET_FREQUENCY_AND_PARAMETERS+(number*10)); // Parameters and Frequency
			printk(KERN_INFO "pnpipcinc ioctl IOCTL_CMD_REG_FREQUENCY %lu counter %d", ioctl_param, number);
			break;
		case IOCTL_CMD_CLEAR_AND_START_COUNTER:
			iowrite8(ioctl_param, pnpi_dev->cs2_mem_addr+CS2_ADDR_OFFSET_CLEAR_START_COUNTER+(number*10));
			printk(KERN_INFO "pnpipcinc ioctl IOCTL_CMD_CLEAR_AND_START_COUNTER %lu counter %d", ioctl_param, number);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static long counter_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
	struct pnpi_device *pnpi_dev = f->private_data;
	int number = pnpi_dev->number;

	switch (ioctl_num)
	{
		case IOCTL_CMD_LEAD_COUNTER_AND_TOF:
			outb(ioctl_param, pnpi_dev->cs0_port+CS0_ADDR_OFFSET_LEADING_AND_TOF);
			printk(KERN_INFO "pnpipcinc ioctl IOCTL_CMD_LEAD_COUNTER_AND_TOF %lu counter %d", ioctl_param, number);
			break;
		case IOCTL_CMD_INVERSE_SIGNAL_COUNTERS:
			outb(ioctl_param, pnpi_dev->cs0_port+CS0_ADDR_OFFSET_INVERTED);
				printk(KERN_INFO "pnpipcinc ioctl IOCTL_CMD_INVERSE_SIGNAL %lu counter %d", ioctl_param, number);
			break;
		case IOCTL_CMD_ALLOW_COUNTERS:
			outb(ioctl_param, pnpi_dev->cs0_port+CS0_ADDR_OFFSET_ALLOW);
			printk(KERN_INFO "pnpipcinc ioctl IOCTL_CMD_FORBID_ALLOW_CLEAR_COUNTER %lu counter %d", ioctl_param, number);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static ssize_t timer_read(struct file *f, char __user * buf, size_t count, loff_t *f_pos)
{
	struct pnpi_device *pnpi_dev = f->private_data;
	int number = pnpi_dev->number;

	unsigned int value = 0;
	char local_buf[256];
	int i;

	for (i=0; i<4; i++)
	{
		value += (ioread8(pnpi_dev->cs2_mem_addr+CS2_ADDR_OFFSET_READ_TIMER+(number*10)+i) & 0xff) << (i*8);
	}

	printk(KERN_INFO "pnpipcinc read %d from timer %d", value, number);
	sprintf(local_buf, "%d", value);
	if (*f_pos >= strlen(local_buf))
		return 0;
	if (*f_pos + count >= strlen(local_buf))
		count = strlen(local_buf) - *f_pos;
	if (copy_to_user(buf, local_buf, count))
		return -EFAULT;

	*f_pos += count;
	printk(KERN_INFO "pnpipcinc read %d from timer %d ok", value, number);
	return count;
}

static ssize_t counter_read(struct file *f, char __user * buf, size_t count, loff_t *f_pos)
{
	struct pnpi_device *pnpi_dev = f->private_data;
	int number = pnpi_dev->number;

	unsigned int value = 0;
	char local_buf[256];
	int i;

	for (i=0; i<2; i++)
	{
		value += (inb(pnpi_dev->cs1_port+CS1_ADDR_OFFSET_READ_COUNTER+(number*8)+i*2) & 0xffff) << (i*16);
	}

	printk(KERN_INFO "pnpipcinc read %d from counter %d", value, number);
	sprintf(local_buf, "%d", value);
	if (*f_pos >= strlen(local_buf))
		return 0;
	if (*f_pos + count >= strlen(local_buf))
		count = strlen(local_buf) - *f_pos;
	if (copy_to_user(buf, local_buf, count))
		return -EFAULT;

	*f_pos += count;
	printk(KERN_INFO "pnpipcinc read %d from counter %d ok", value, number);
	return count;
}


static ssize_t timer_write(struct file *f, const char __user * buf, size_t count, loff_t *f_pos)
{
	struct pnpi_device *pnpi_dev = f->private_data;
	int number = pnpi_dev->number;

	unsigned int value;
	char local_buf[256];
	int i;

	if (count > 255)
		count = 255;
	if(copy_from_user(local_buf, buf, count))
		return -EFAULT;
	local_buf[count] = '\0';
	if (kstrtouint(local_buf, 0, &value))
		return -EFAULT;

	*f_pos += count;

	printk(KERN_INFO "pnpipcinc write %d to timer %d", value, number);
	for (i=0; i<4; i++)
	{
		iowrite8((value >> i*8) & 0xff, pnpi_dev->cs2_mem_addr+CS2_ADDR_OFFSET_WRITE_PRESET+(number*10)+i);
	}
	printk(KERN_INFO "pnpipcinc write %d to timer %d ok", value, number);

	return count;
}

static int pnpi_release(struct inode *inode, struct file* f)
{
	if (f->private_data)
	{
		f->private_data = NULL;
	}
	return 0;
}

static struct file_operations counter_fops = {
	.owner =		THIS_MODULE,
	.read =			counter_read,
	.unlocked_ioctl = 	counter_ioctl,
	.open =			pnpi_open,
	.release = 		pnpi_release,
};

static struct file_operations timer_fops = {
	.owner =		THIS_MODULE,
	.read =			timer_read,
	.write =	 	timer_write,
	.unlocked_ioctl = 	timer_ioctl,
	.open =			pnpi_open,
	.release = 		pnpi_release,
};

int create_pnpi_device(int serial_number, char *type, int number,
		struct pnpi_device *pnpi_dev,
		unsigned long cs0_port, unsigned long cs1_port,
		void __iomem *cs2_mem_addr, void __iomem *cs3_mem_addr)
{
	int ret;
	char pnpi_device_name[128];
	char pnpi_device_class_name[128];

	pnpi_dev->number = number;
	strcpy(pnpi_dev->type, type);
	pnpi_dev->cs0_port = cs0_port;
	pnpi_dev->cs1_port = cs1_port;
	pnpi_dev->cs2_mem_addr = cs2_mem_addr;
	pnpi_dev->cs3_mem_addr = cs3_mem_addr;

	ret = sprintf(pnpi_device_name, "pnpipcinc%d%s%d", serial_number, type, number);
	ret = sprintf(pnpi_device_class_name, "pnpipcinc%d%s%dclass", serial_number, type, number);

	ret = alloc_chrdev_region(&pnpi_dev->majorminor, 0, 256, "pnpipnpi_device");
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc allocate pnpi_device device numbers failed");
		return ret;
	}
	printk(
			KERN_INFO "pnpipcinc allocated %s character device with %d major and %d minor numbers",
			pnpi_device_name,
			MAJOR(pnpi_dev->majorminor),
			MINOR(pnpi_dev->majorminor)
			);

	pnpi_dev->pnpi_device_class = class_create(THIS_MODULE, pnpi_device_class_name);
	if (pnpi_dev->pnpi_device_class == NULL)
		goto class_fail;

	printk(
			KERN_INFO "pnpipcinc created %s character device class",
			pnpi_device_class_name);

	if(device_create(pnpi_dev->pnpi_device_class, NULL, pnpi_dev->majorminor, NULL, pnpi_device_name) == NULL)
		goto device_fail;

	printk(
			KERN_INFO "pnpipcinc created %s character device",
			pnpi_device_name);

	switch(type[0])
	{
		case 'c':
			cdev_init(&pnpi_dev->cdev, &counter_fops);
			pnpi_dev->cdev.ops = &counter_fops;
			break;
		case 't':
			cdev_init(&pnpi_dev->cdev, &timer_fops);
			pnpi_dev->cdev.ops = &timer_fops;
			break;
	}
	pnpi_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&pnpi_dev->cdev, pnpi_dev->majorminor, 256);
	if (ret < 0)
		goto cdev_fail;

	printk(
			KERN_INFO "pnpipcinc allocated and added %s character device representation",
			pnpi_device_name);

	return 0;

cdev_fail:
	printk(KERN_ERR "pnpipcinc create pnpi_device device class failed");
       	device_destroy(pnpi_dev->pnpi_device_class, pnpi_dev->majorminor);
       	printk(KERN_INFO "pnpipcinc char devices destroyed");
device_fail:
	class_destroy(pnpi_dev->pnpi_device_class);
	printk(KERN_INFO "pnpipcinc class of device destroyed");
class_fail:
	unregister_chrdev_region(pnpi_dev->majorminor, 256);
	printk(KERN_INFO "pnpipcinc char device unregistered");
	return -1;
}

void remove_pnpi_device(struct pnpi_device *pnpi_dev)
{
	cdev_del(&pnpi_dev->cdev);
	printk(KERN_INFO "pnpipcinc char devices representations deleted");

	device_destroy(pnpi_dev->pnpi_device_class, pnpi_dev->majorminor);
	printk(KERN_INFO "pnpipcinc char devices destroyed");

	class_destroy(pnpi_dev->pnpi_device_class);
	printk(KERN_INFO "pnpipcinc classes of devices destroyed");

	unregister_chrdev_region(pnpi_dev->majorminor, 256);
	printk(KERN_INFO "pnpipcinc char devices unregistered");
}

static int sn(struct pci_dev *pdev)
{
	/* read pci card serial number from eeprom
	 * */
	u32 eeprom_magic_addr = 0x00a00003;
	u32 sn = 0;
	int count = 0;
	pci_write_config_dword(pdev, 0x4c, eeprom_magic_addr);

	/* wait for eeprom */
	while((sn==0)||(count < 100)){
	    pci_read_config_dword(pdev, 0x50, &sn);
	    mdelay(1);
	    count++;
	}
	printk(KERN_INFO "pnpipcinc got serial number %d or 0x%x hex", sn, sn&0xffffffff);
	return sn;
}

static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret;
	int serial_number;
	struct pnpi_pci_data_t *privdata;

	void __iomem *cs2_mem_addr;
	void __iomem *cs3_mem_addr;
	unsigned long cs0_port;
	unsigned long cs1_port;


	printk(KERN_INFO "pnpipcinc pci probe :");

	ret = pci_enable_device(pdev);

	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc pci enable failed");
		return ret;
	}
	printk(KERN_INFO "pnpipcinc pci enabled");

	ret = pci_request_regions(pdev, "pnpipcinc");
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc pci request regions failed");
		pci_disable_device(pdev);
		return ret;
	}

	pci_set_master(pdev);

	cs0_port = pci_resource_start(pdev, 2);
	cs1_port = pci_resource_start(pdev, 3);
	cs2_mem_addr = pci_ioremap_bar(pdev, 4);
	cs3_mem_addr = pci_ioremap_bar(pdev, 5);

	printk(KERN_INFO "pnpipcinc pci request regions ok");

	serial_number = sn(pdev);

	if ((privdata = kcalloc(1, sizeof(*privdata), GFP_KERNEL)) == NULL)
	{
		printk(KERN_ERR "pnpipcinc can not allocate pci privdata struct memory");
		goto fail;
	}

	ret = create_pnpi_device(serial_number, "counter", 0, &privdata->counter0, cs0_port, cs1_port, cs2_mem_addr, cs3_mem_addr);
	if (ret < 0)
		goto fail;
	ret = create_pnpi_device(serial_number, "counter", 1, &privdata->counter1, cs0_port, cs1_port, cs2_mem_addr, cs3_mem_addr);
	if (ret < 0)
		goto fail;
	ret = create_pnpi_device(serial_number, "timer", 0, &privdata->timer0, cs0_port, cs1_port, cs2_mem_addr, cs3_mem_addr);
	if (ret < 0)
		goto fail;
	ret = create_pnpi_device(serial_number, "timer", 1, &privdata->timer1, cs0_port, cs1_port, cs2_mem_addr, cs3_mem_addr);
	if (ret < 0)
		goto fail;

	pci_set_drvdata(pdev, privdata);

	return 0;

fail:
	pci_release_regions(pdev);
	printk(KERN_INFO "pnpipcinc pci regions released");
	pci_disable_device(pdev);
	printk(KERN_INFO "pnpipcinc pci device disabled");
}

void remove (struct pci_dev *pdev)
{

	struct pnpi_pci_data_t *privdata = pci_get_drvdata(pdev);

	printk(KERN_INFO "pnpipcinc remove");

	remove_pnpi_device(&privdata->counter0);
	remove_pnpi_device(&privdata->counter1);
	remove_pnpi_device(&privdata->timer0);
	remove_pnpi_device(&privdata->timer1);

	kfree(privdata);

	pci_release_regions(pdev);
	printk(KERN_INFO "pnpipcinc pci regions released");

	pci_disable_device(pdev);
	printk(KERN_INFO "pnpipcinc pci device disabled");
}


static struct pci_driver pci_driver = {
	.name = "pnpipcinc",
	.id_table = pnpipcinc_ids,
	.probe = probe,
	.remove = remove,
};

static int __init pnpipcinc_init(void)
{
	int res;
	res = pci_register_driver(&pci_driver);
	printk(KERN_INFO "pnpipcinc register result %d", res);
	return res;
}

static void __exit pnpipcinc_exit(void)
{
	printk(KERN_INFO "pnpipcinc exit start");

	pci_unregister_driver(&pci_driver);
	printk(KERN_INFO "pnpipcinc unloaded");
}


module_init(pnpipcinc_init);
module_exit(pnpipcinc_exit);
MODULE_AUTHOR("Mikhail Golubev  <mgolubev86@gmail.com>");
MODULE_DESCRIPTION("pnpipcinc: Petersburg Nuclear Physics Institute PCI Neutron Counter device driver");
MODULE_LICENSE("GPL");
