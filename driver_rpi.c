#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kfifo.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define GPIO_BASE 512
#define LEDS_MASK 0x3f
#define GPIO_SPEAKER (GPIO_BASE + 4)

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

static ssize_t speaker_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *ppos)
{
	char value;

	if (!count)
		return 0;
	if (copy_from_user(&value, buf, 1))
		return -EFAULT;

	gpio_set_value(GPIO_SPEAKER, value != '0');
	*ppos += 1;
	return 1;
}

static const struct file_operations speaker_fops = {
	.owner = THIS_MODULE,
	.write = speaker_write,
};

static struct miscdevice speaker_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "speaker",
	.fops = &speaker_fops,
};

static unsigned int debounce_ms = 50;
module_param(debounce_ms, uint, 0444);
MODULE_PARM_DESC(debounce_ms, "Intervalo antirrebote en milisegundos");

struct button {
	unsigned int gpio;
	int irq;
	char key;
	unsigned long last_press;
};

static struct button buttons[] = {
	{ .gpio = GPIO_BASE + 2, .key = '1' },
	{ .gpio = GPIO_BASE + 3, .key = '2' },
};

static DEFINE_KFIFO(buttons_fifo, char, 64);
static DEFINE_SPINLOCK(buttons_lock);
static DEFINE_MUTEX(buttons_read_mutex);
static DECLARE_WAIT_QUEUE_HEAD(buttons_wait);
static struct semaphore buttons_open_sem;

static int buttons_open(struct inode *inode, struct file *file)
{
	if (file->f_flags & O_NONBLOCK) {
		if (down_trylock(&buttons_open_sem))
			return -EBUSY;
	} else if (down_interruptible(&buttons_open_sem)) {
		return -ERESTARTSYS;
	}
	return 0;
}

static int buttons_release(struct inode *inode, struct file *file)
{
	up(&buttons_open_sem);
	return 0;
}

static irqreturn_t button_irq(int irq, void *dev_id)
{
	struct button *button = dev_id;
	unsigned long now = jiffies;

	if (time_before(now, button->last_press + msecs_to_jiffies(debounce_ms)))
		return IRQ_HANDLED;
	button->last_press = now;
	return IRQ_WAKE_THREAD;
}

static irqreturn_t button_thread(int irq, void *dev_id)
{
	struct button *button = dev_id;
	unsigned long flags;
	bool added;

	spin_lock_irqsave(&buttons_lock, flags);
	added = kfifo_put(&buttons_fifo, button->key);
	spin_unlock_irqrestore(&buttons_lock, flags);
	if (added)
		wake_up_interruptible(&buttons_wait);
	return IRQ_HANDLED;
}

static bool buttons_peek(char *key)
{
	unsigned long flags;
	bool available;

	spin_lock_irqsave(&buttons_lock, flags);
	available = kfifo_peek(&buttons_fifo, key);
	spin_unlock_irqrestore(&buttons_lock, flags);
	return available;
}

static ssize_t buttons_read(struct file *file, char __user *buf,
			    size_t count, loff_t *ppos)
{
	unsigned long flags;
	char key;
	ssize_t ret;

	if (!count)
		return 0;
	if (file->f_flags & O_NONBLOCK) {
		if (!mutex_trylock(&buttons_read_mutex))
			return -EAGAIN;
	} else if (mutex_lock_interruptible(&buttons_read_mutex)) {
		return -ERESTARTSYS;
	}

	if (file->f_flags & O_NONBLOCK) {
		if (!buttons_peek(&key)) {
			ret = -EAGAIN;
			goto unlock;
		}
	} else if (wait_event_interruptible(buttons_wait, buttons_peek(&key))) {
		ret = -ERESTARTSYS;
		goto unlock;
	}

	if (copy_to_user(buf, &key, 1)) {
		ret = -EFAULT;
		goto unlock;
	}
	spin_lock_irqsave(&buttons_lock, flags);
	kfifo_skip(&buttons_fifo);
	spin_unlock_irqrestore(&buttons_lock, flags);
	*ppos += 1;
	ret = 1;
unlock:
	mutex_unlock(&buttons_read_mutex);
	return ret;
}

static const struct file_operations buttons_fops = {
	.owner = THIS_MODULE,
	.open = buttons_open,
	.release = buttons_release,
	.read = buttons_read,
};

static struct miscdevice buttons_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "buttons",
	.fops = &buttons_fops,
};

static int buttons_init(void)
{
	int i, ret;

	sema_init(&buttons_open_sem, 1);
	for (i = 0; i < ARRAY_SIZE(buttons); i++) {
		ret = gpio_request_one(buttons[i].gpio, GPIOF_IN,
				       "driver_rpi_button");
		if (ret)
			goto free_previous;
		ret = gpio_to_irq(buttons[i].gpio);
		if (ret < 0)
			goto free_current;
		buttons[i].irq = ret;
		buttons[i].last_press = jiffies - msecs_to_jiffies(debounce_ms);
		ret = request_threaded_irq(buttons[i].irq, button_irq, button_thread,
					   IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					   "driver_rpi_button", &buttons[i]);
		if (ret)
			goto free_current;
	}
	return 0;

free_current:
	gpio_free(buttons[i].gpio);
free_previous:
	while (i--) {
		free_irq(buttons[i].irq, &buttons[i]);
		gpio_free(buttons[i].gpio);
	}
	return ret;
}

static void buttons_cleanup(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(buttons); i++) {
		free_irq(buttons[i].irq, &buttons[i]);
		gpio_free(buttons[i].gpio);
	}
}

static int __init driver_rpi_init(void)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(led_gpios); i++) {
		ret = gpio_request_one(led_gpios[i], GPIOF_OUT_INIT_LOW,
				       "driver_rpi_led");
		if (ret)
			goto free_leds;
	}
	ret = gpio_request_one(GPIO_SPEAKER, GPIOF_OUT_INIT_LOW,
			       "driver_rpi_speaker");
	if (ret)
		goto free_leds;
	ret = buttons_init();
	if (ret)
		goto free_speaker;
	ret = misc_register(&leds_device);
	if (ret)
		goto free_buttons;
	ret = misc_register(&speaker_device);
	if (ret)
		goto deregister_leds;
	ret = misc_register(&buttons_device);
	if (ret)
		goto deregister_speaker;

	pr_info("driver_rpi: cargado\n");
	return 0;

deregister_speaker:
	misc_deregister(&speaker_device);
deregister_leds:
	misc_deregister(&leds_device);
free_buttons:
	buttons_cleanup();
free_speaker:
	gpio_free(GPIO_SPEAKER);
free_leds:
	while (i--)
		gpio_free(led_gpios[i]);
	return ret;
}

static void __exit driver_rpi_exit(void)
{
	int i;

	misc_deregister(&buttons_device);
	misc_deregister(&speaker_device);
	misc_deregister(&leds_device);
	buttons_cleanup();
	gpio_set_value(GPIO_SPEAKER, 0);
	gpio_free(GPIO_SPEAKER);
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
