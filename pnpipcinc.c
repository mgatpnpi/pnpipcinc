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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "pnpipcinc.h"

MODULE_AUTHOR("Golubev Mikhail <mgolubev86@gmail.com>");
MODULE_DESCRIPTION("pnpipcinc: Petersburg Nuclear Physics Institute PCI Neutron Counter device driver");
MODULE_LICENSE("GPL");

static struct pci_device_id pnpipcinc_ids[] = {
	{ PCI_DEVICE( 0x10b5, 0x90f1) },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, pnpipcinc_ids);


static int sn(struct pci_dev *pdev)
{
	/* read pci card serial number from eeprom
	 * */
	u32 eeprom_magic_addr = 0x00a00003;
	u32 sn;
	pci_write_config_dword(pdev, 0x4c, eeprom_magic_addr);
	pci_read_config_dword(pdev, 0x50, &sn);
	printk(KERN_INFO "pnpipcinc got serial number %d or %x hex", sn, sn);
	return sn;
}


dev_t countermajorminor1;
dev_t countermajorminor2;

struct cdev * cdevcounter1;
struct cdev * cdevcounter2;

void __iomem *cs2_mem_addr;
void __iomem *cs3_mem_addr;
long cs0_port;
long cs1_port;

unsigned int counters[2];

int counter_open(struct inode *inode, struct file * f)
{
	unsigned int minor = iminor(inode);
	counters[minor - 1] = minor;
	f->private_data = &counters[minor-1];

	return 0;
}

long counter_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
	unsigned int minor = *(unsigned int*)f->private_data;
	if ((ioctl_param & IOCTL_CMD_SET_LEADING_COUNTER) > 0)
	{
		outb(cs0_port, (minor-1) << 1);
	}
	iowrite8(0, cs2_mem_addr+10+((minor - 1)*10)); // Flush
	iowrite8(ioctl_param & 127, cs2_mem_addr+9+((minor - 1)*10)); // Parameters and Frequency
	return 0;
}

ssize_t counter_read(struct file *f, char __user * buf, size_t count, loff_t *f_pos)
{
	unsigned int minor = *(unsigned int*)f->private_data;
	unsigned int value = ioread32(cs2_mem_addr+15+((minor-1)*10));
	char local_buf[256];
	printk(KERN_INFO "pnpipcinc read %d from counter %d", value, minor);
	sprintf(local_buf, "%d", value);
	if (copy_to_user(buf, local_buf, strlen(local_buf)))
	{
		return -EFAULT;
	}
	printk(KERN_INFO "pnpipcinc read %d from counter %d ok", value, minor);
	return 0;
}

ssize_t counter_write(struct file *f, const char __user * buf, size_t count, loff_t *f_pos)
{
	unsigned int minor = *(unsigned int*)f->private_data;
	unsigned int value;
	char local_buf[256];
	if(copy_from_user(local_buf, buf, strlen(buf)))
	{
		return -EFAULT;
	}
	if (kstrtouint(local_buf, 0, &value))
	{
		return -EFAULT;
	}
	printk(KERN_INFO "pnpipcinc write %d to counter %d", value, minor);
	iowrite32(value, cs2_mem_addr+11+((minor-1)*10));
	printk(KERN_INFO "pnpipcinc write %d to counter %d ok", value, minor);
	
	return 0;
}

int counter_release(struct inode *inode, struct file* f)
{
	if (f->private_data)
	{
		f->private_data = NULL;
	}
	return 0;
}

static struct file_operations counter_fops = {
	owner :		THIS_MODULE,
	read :		counter_read,
	write : 	counter_write,
	unlocked_ioctl : 	counter_ioctl,
	open :		counter_open,
	release : 	counter_release,
};

static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret;
	int serial_number;
	char counter1name[128];
	char counter2name[128];

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

	printk(KERN_INFO "pnpipcinc pci request regions ok");
	pci_set_master(pdev);

	serial_number = sn(pdev);

	ret = sprintf(counter1name, "pnpipcinc%dcounter1", serial_number);
	ret = sprintf(counter2name, "pnpipcinc%dcounter2", serial_number);

	ret = alloc_chrdev_region(&countermajorminor1, 1, 1, counter1name);
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc allocate counter1 device numbers failed");

		pci_release_regions(pdev);
		printk(KERN_INFO "pnpipcinc pci regions released");
	
		pci_disable_device(pdev);
		printk(KERN_INFO "pnpipcinc pci device disabled");
		return ret;
	}
	printk(
			KERN_INFO "pnpipcinc allocated %s character device with %d major and %d minor numbers",
			counter1name,
			MAJOR(countermajorminor1),
			MINOR(countermajorminor1)
			);

	ret = alloc_chrdev_region(&countermajorminor2, 2, 1, counter2name);
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc allocate counter2 device numbers failed");

		unregister_chrdev_region(countermajorminor1, 1);
		printk(KERN_INFO "pnpipcinc char devices unregistered");

		pci_release_regions(pdev);
		printk(KERN_INFO "pnpipcinc pci regions released");
	
		pci_disable_device(pdev);
		printk(KERN_INFO "pnpipcinc pci device disabled");
		return ret;
	}
	printk(
			KERN_INFO "pnpipcinc allocated %s character device with %d major and %d minor numbers",
			counter2name,
			MAJOR(countermajorminor2),
			MINOR(countermajorminor2)
			);

	cdevcounter1 = cdev_alloc();
	cdev_init(cdevcounter1, &counter_fops);
	cdevcounter1->owner = THIS_MODULE;
	cdevcounter1->ops = &counter_fops;
	ret = cdev_add(cdevcounter1, countermajorminor1, 1);
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc add counter1 character device failed");

		unregister_chrdev_region(countermajorminor1, 1);
		unregister_chrdev_region(countermajorminor2, 1);
		printk(KERN_INFO "pnpipcinc char devices unregistered");
	
		pci_release_regions(pdev);
		printk(KERN_INFO "pnpipcinc pci regions released");
	
		pci_disable_device(pdev);
		printk(KERN_INFO "pnpipcinc pci device disabled");
		return ret;
	}
	printk(
			KERN_INFO "pnpipcinc allocated and added %s character device representation",
			counter1name);

	cdevcounter2 = cdev_alloc();
	cdev_init(cdevcounter2, &counter_fops);
	cdevcounter2->owner = THIS_MODULE;
	cdevcounter2->ops = &counter_fops;
	ret = cdev_add(cdevcounter2, countermajorminor2, 1);
	if (ret < 0)
	{
		printk(KERN_ERR "pnpipcinc add counter2 character device failed");

		cdev_del(cdevcounter1);
		printk(KERN_INFO "pnpipcinc char devices representations deleted");

		unregister_chrdev_region(countermajorminor1, 1);
		unregister_chrdev_region(countermajorminor2, 1);
		printk(KERN_INFO "pnpipcinc char devices unregistered");
	
		pci_release_regions(pdev);
		printk(KERN_INFO "pnpipcinc pci regions released");
	
		pci_disable_device(pdev);
		printk(KERN_INFO "pnpipcinc pci device disabled");
		return ret;
	}
	printk(
			KERN_INFO "pnpipcinc allocated and added %s character device representation",
			counter1name);

	cs0_port = pci_resource_start(pdev, 2);
	cs1_port = pci_resource_start(pdev, 3);
	cs2_mem_addr = pci_ioremap_bar(pdev, 4);
	cs3_mem_addr = pci_ioremap_bar(pdev, 5);
	
	return 0;
}

void remove (struct pci_dev *pdev)
{
	printk(KERN_INFO "pnpipcinc remove");

	cdev_del(cdevcounter1);
	cdev_del(cdevcounter2);
	printk(KERN_INFO "pnpipcinc char devices representations deleted");

	unregister_chrdev_region(countermajorminor1, 1);
	unregister_chrdev_region(countermajorminor2, 1);
	printk(KERN_INFO "pnpipcinc char devices unregistered");

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

static int pnpipcinc_init(void)
{
	int res;
	res = pci_register_driver(&pci_driver);
	printk(KERN_INFO "pnpipcinc register result %d", res);
	return res;
}

static void pnpipcinc_exit(void)
{
	pci_unregister_driver(&pci_driver);
	printk(KERN_INFO "pnpipcinc unloaded");
}


module_init(pnpipcinc_init);
module_exit(pnpipcinc_exit);
