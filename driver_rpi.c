#include <linux/init.h>
#include <linux/module.h>

static int __init driver_rpi_init(void)
{
	pr_info("driver_rpi: cargado\n");
	return 0;
}

static void __exit driver_rpi_exit(void)
{
	pr_info("driver_rpi: descargado\n");
}

module_init(driver_rpi_init);
module_exit(driver_rpi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manuel");
MODULE_DESCRIPTION("Driver para la placa auxiliar Raspberry Pi");
