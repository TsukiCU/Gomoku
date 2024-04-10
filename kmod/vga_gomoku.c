/* * Device driver for the VGA video generator
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod vga_gomoku.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree vga_gomoku.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "vga_gomoku.h"

#define DRIVER_NAME "vga_gomoku"

/* Device registers */
// #define CIR_RED(x) (x)
// #define CIR_GREEN(x) ((x)+1)
// #define CIR_BLUE(x) ((x)+2)
// #define CIR_XH(x) ((x)+3)
// #define CIR_XL(x) ((x)+4)
// #define CIR_YH(x) ((x)+5)
// #define CIR_YL(x) ((x)+6)
// #define CIR_R(x) ((x)+7)
// #define BG_RED(x) ((x)+8)
// #define BG_GREEN(x) ((x)+9)
// #define BG_BLUE(x) ((x)+10)

/*
 * Information about our device
 */
struct vga_gomoku_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

/*
 * Write segments of a single digit
 * Assumes digit is in range and the device information has been set up
 */
static void write_data(vga_gomoku_arg_t *arg)
{
	for(int i=0;i<8;++i)
		iowrite8(vla.param[i], dev.virtbase+i);
}

/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long vga_gomoku_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	vga_gomoku_arg_t vla;

	switch (cmd) {
	case VGA_GOMOKU_WRITE:
		if (copy_from_user(&vla, (vga_gomoku_arg_t *) arg,
				   sizeof(vga_gomoku_arg_t)))
			return -EACCES;
		write_data(&vla);
		// write_circle_color(&vla.c_color);
		// write_circle(&vla.circle);
		// write_bg_color(&vla.bg_color);
		break;

	// case vga_gomoku_READ:
	//   	vla.c_color = dev.c_color;
	// 	vla.circle = dev.circle;
	// 	vla.bg_color = dev.bg_color;
	// 	if (copy_to_user((vga_gomoku_arg_t *) arg, &vla,
	// 			 sizeof(vga_gomoku_arg_t)))
	// 		return -EACCES;
	// 	break;
	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations vga_gomoku_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = vga_gomoku_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice vga_gomoku_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &vga_gomoku_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init vga_gomoku_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/vga_gomoku */
	ret = misc_register(&vga_gomoku_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}

	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&vga_gomoku_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int vga_gomoku_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&vga_gomoku_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id vga_gomoku_of_match[] = {
	{ .compatible = "csee4840,vga_gomoku-1.0" },
	{},
};
MODULE_DEVICE_TABLE(of, vga_gomoku_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver vga_gomoku_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(vga_gomoku_of_match),
	},
	.remove	= __exit_p(vga_gomoku_remove),
};

/* Called when the module is loaded: set things up */
static int __init vga_gomoku_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&vga_gomoku_driver, vga_gomoku_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit vga_gomoku_exit(void)
{
	platform_driver_unregister(&vga_gomoku_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(vga_gomoku_init);
module_exit(vga_gomoku_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
MODULE_DESCRIPTION("VGA ball driver");
