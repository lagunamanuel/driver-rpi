#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define GPIO_BASE 512
#define LEDS_MASK 0x3f

static const unsigned int led_gpios[] = {
	GPIO_BASE + 9, GPIO_BASE + 10, GPIO_BASE + 11,
	GPIO_BASE + 17, GPIO_BASE + 22, GPIO_BASE + 27,
};

static DEFINE_MUTEX(leds_mutex);
static u8 leds_state;

static ssize_t leds_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	u8 state;

	mutex_lock(&leds_mutex);
	state = leds_state;
	mutex_unlock(&leds_mutex);

	return simple_read_from_buffer(buf, count, ppos, &state, 1);
}

static ssize_t leds_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	u8 command, mask;
	int i;

	if (!count)
		return 0;
	if (copy_from_user(&command, buf, 1))
		return -EFAULT;

	mask = command & LEDS_MASK;
	mutex_lock(&leds_mutex);
	switch (command >> 6) {
	case 0:
		leds_state = mask;
		break;
	case 1:
		leds_state |= mask;
		break;
	case 2:
		leds_state &= ~mask;
		break;
	case 3:
		leds_state ^= mask;
		break;
	}
	for (i = 0; i < ARRAY_SIZE(led_gpios); i++)
		gpio_set_value(led_gpios[i], !!(leds_state & BIT(i)));
	mutex_unlock(&leds_mutex);

	*ppos += 1;
	return 1;
}

static const struct file_operations leds_fops = {
	.owner = THIS_MODULE,
	.read = leds_read,
	.write = leds_write,
};

static struct miscdevice leds_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "leds",
	.fops = &leds_fops,
};

static int __init driver_rpi_init(void)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(led_gpios); i++) {
		ret = gpio_request_one(led_gpios[i], GPIOF_OUT_INIT_LOW,
				       "driver_rpi_led");
		if (ret)
			goto free_leds;
	}
	ret = misc_register(&leds_device);
	if (ret)
		goto free_leds;

	pr_info("driver_rpi: cargado\n");
	return 0;

free_leds:
	while (i--)
		gpio_free(led_gpios[i]);
	return ret;
}

static void __exit driver_rpi_exit(void)
{
	int i;

	misc_deregister(&leds_device);
	for (i = 0; i < ARRAY_SIZE(led_gpios); i++) {
		gpio_set_value(led_gpios[i], 0);
		gpio_free(led_gpios[i]);
	}
	pr_info("driver_rpi: descargado\n");
}

module_init(driver_rpi_init);
module_exit(driver_rpi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manuel");
MODULE_DESCRIPTION("Driver para la placa auxiliar Raspberry Pi");
