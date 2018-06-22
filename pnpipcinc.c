/* pnpipcinc: Petersburg Nuclear Physics Institute PCI Neutron Counter device driver */
/* 
 * Author: Mikhail Golubev
 * Organization: LO, Gatchina, Orlova Roscha, NRC KI PNPI
 * hardware: Altera Cyclon v2 + plx9030
 * since: 2018
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
#include <asm/io.h>


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

static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret;
	int serial_number;
	int i,j;
	void __iomem *addr;
	long port;
	int value[16];
	printk(KERN_INFO "pnpipcinc pci probe :");
	ret = pci_enable_device(pdev);

	if (ret < 0)
	{
		printk(KERN_INFO "pnpipcinc pci enable failed");
		return ret;
	}
	printk(KERN_INFO "pnpipcinc pci enabled");

	ret = pci_request_regions(pdev, "pnpipcinc");
	if (ret < 0)
	{
		printk(KERN_INFO "pnpipcinc pci request regions failed");
		pci_disable_device(pdev);
		return ret;
	}
	printk(KERN_INFO "pnpipcinc pci request regions ok");
	pci_set_master(pdev);

	serial_number = sn(pdev);

	addr = pci_ioremap_bar(pdev, 0);
	port = pci_resource_start(pdev, 2);
	
	for (i=0;i<8;i++)
	{
		for (j=0;j<16;j++)
		{
			value[j] = ioread8(addr+j+i*16);
		//	value[j] = inb(port+j+i*16);
		//	pci_read_config_byte(pdev, i*16+j, value+j);
		}
		printk(KERN_INFO "reg %x0 value %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x", i, value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7], value[8], value[9], value[10], value[11], value[12], value[13], value[14], value[15]);
	}
	
	value[0] = inb(port+29);
	printk(KERN_INFO "reg 29 value %x %d", value[0], value[0]);

	port = pci_resource_start(pdev, 3);
	value[0] = inw(port+28);
	printk(KERN_INFO "reg 28 value %x %d", value[0], value[0]);

	outw(123, port+24);
	value[0] = inw(port+24);
	printk(KERN_INFO "reg 24 value %x %d", value[0], value[0]);
	value[0] = inw(port+26);
	printk(KERN_INFO "reg 26 value %x %d", value[0], value[0]);

	addr = pci_ioremap_bar(pdev, 4);
	value[0] = ioread8(addr+30);
	printk(KERN_INFO "reg 30 value %x %d", value[0], value[0]);
	
	/*
	for (i=0;i<64;i++)
	{
		for (j=0;j<4;j++)
		{
			value[j] = ioread32(addr+j*4+i*16);
		//	value[j] = inb(port+j+i*16);
		//	pci_read_config_byte(pdev, i*16+j, value+j);
		}
		printk(KERN_INFO "reg %x0 value %.12d %.12d %.12d %.12d", i, value[0], value[1], value[2], value[3]);
	}
	*/
	return 0;
}

void remove (struct pci_dev *pdev)
{
	printk(KERN_INFO "pnpipcinc remove");
	pci_release_regions(pdev);
	pci_disable_device(pdev);
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
