
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>

#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-id.h>

#include <asm/uaccess.h>
#include <asm/io.h>
//#include <asm/au1000.h>

#include <mach/gp_board.h>
#include <mach/gp_pin_grp.h>
#include <mach/gp_pin_grp.h>
#include <mach/gp_gpio.h>
#include <mach/gp_apbdma.h>


// CLK
#define GPIO3_18_CHN	3
#define GPIO3_18_PIN	18
#define GPIO3_18_GID	36
#define GPIO3_18_FUN	0
#define GPIO3_18_IDX	((GPIO3_18_CHN<<24)|(GPIO3_18_FUN<<16)|(GPIO3_18_GID<<8)|GPIO3_18_PIN)

// DATA
#define GPIO3_19_CHN	3
#define GPIO3_19_PIN	19
#define GPIO3_19_GID	37
#define GPIO3_19_FUN	0
#define GPIO3_19_IDX	((GPIO3_19_CHN<<24)|(GPIO3_19_FUN<<16)|(GPIO3_19_GID<<8)|GPIO3_19_PIN)

#define DELAY			20

/*
static unsigned int i2c_debug = 3;

#define bit_dbg(level, dev, format, args...) \
	do { \
		if (i2c_debug >= level) \
			dev_dbg(dev, format, ##args); \
	} while (0)
*/


#define I2C0_BIT_GPDATA	0x00000002
#define I2C0_BIT_GPCLK		0x00000004
#define I2C1_BIT_GPDATA	0x00000008
#define I2C1_BIT_GPCLK		0x00000010
#define I2C2_BIT_GPDATA	0x00000020
#define I2C2_BIT_GPCLK		0x00000040

#define setsda(adap, val)	adap->setsda(adap->data, val)
#define setscl(adap, val)	adap->setscl(adap->data, val)
#define getsda(adap)		adap->getsda(adap->data)
#define getscl(adap)		adap->getscl(adap->data)


static int hScl = 0;
static int hSda = 0;

static struct gpl8300_i2c_data {
	unsigned int sda_line;
	unsigned int scl_line;
} gpio_data;



static void output_pin_setting(void)
{
	gp_gpio_set_function(hScl, GPIO_FUNC_GPIO);
	gp_gpio_set_function(hSda, GPIO_FUNC_GPIO);

	gp_gpio_set_direction(hScl, GPIO_DIR_OUTPUT);
	gp_gpio_set_direction(hSda, GPIO_DIR_OUTPUT);

	gp_gpio_set_pullfunction(hScl, GPIO_PULL_FLOATING);
	gp_gpio_set_pullfunction(hSda, GPIO_PULL_FLOATING);

	gp_gpio_set_output(hScl, 1, 0);
	gp_gpio_set_output(hSda, 1, 0);

	gp_gpio_enable_irq(hScl, 0);
	gp_gpio_enable_irq(hSda, 0);

	gp_gpio_set_value(hScl, 1);
	gp_gpio_set_value(hSda, 1);
}

static void gpl8300_bit_setscl(void *data, int val)
{
	if (val)	gp_gpio_set_output(hScl, 1, 0);
	else		gp_gpio_set_output(hScl, 0, 0);
}

static void gpl8300_bit_setsda(void *data, int val)
{
	if (val)	gp_gpio_set_output(hSda, 1, 0);
	else		gp_gpio_set_output(hSda, 0, 0);
}

static int gpl8300_bit_getscl(void *data)
{
	int val = 1;

	gp_gpio_set_direction(hScl, GPIO_DIR_INPUT);
	gp_gpio_get_value(hScl, &val);
	gp_gpio_set_direction(hScl, GPIO_DIR_OUTPUT);

	return val;
}

static int gpl8300_bit_getsda(void *data)
{
	int val = 1;

	gp_gpio_set_direction(hSda, GPIO_DIR_INPUT);
	gp_gpio_get_value(hSda, &val);

	if (val)	return 1;
	else		return 0;
}

static inline void sdalo(struct i2c_algo_bit_data *adap)
{
	setsda(adap, 0);
	udelay((adap->udelay + 1) / 2);
}

static inline void sdahi(struct i2c_algo_bit_data *adap)
{
	setsda(adap, 1);
	udelay((adap->udelay + 1) / 2);
}

static inline void scllo(struct i2c_algo_bit_data *adap)
{
	setscl(adap, 0);
	udelay(adap->udelay / 2);
}

static int sclhi(struct i2c_algo_bit_data *adap)
{
	unsigned long start;

	setscl(adap, 1);

	if (!adap->getscl)
		goto done;

	start = jiffies;
	while (!getscl(adap)) {
		if (time_after(jiffies, start + adap->timeout))
			return -ETIMEDOUT;
		cond_resched();
	}

	if (jiffies != start)
		pr_debug("i2c-algo-bit: needed %ld jiffies for SCL to go high\n", jiffies - start);


done:
	udelay(adap->udelay);
	return 0;
}

static void i2c_start(struct i2c_algo_bit_data *adap)
{
	setsda(adap, 0);
	udelay(adap->udelay);
	scllo(adap);
}

static void i2c_repstart(struct i2c_algo_bit_data *adap)
{
	sdahi(adap);
	sclhi(adap);
	setsda(adap, 0);
	udelay(adap->udelay);
	scllo(adap);
}

static void i2c_stop(struct i2c_algo_bit_data *adap)
{
	sdalo(adap);
	sclhi(adap);
	setsda(adap, 1);
	udelay(adap->udelay);
}

static int i2c_outb(struct i2c_adapter *i2c_adap, unsigned char c)
{
	int i;
	int sb;
	int ack;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 1;
		setsda(adap, sb);
		udelay((adap->udelay + 1) / 2);
		if (sclhi(adap) < 0) {
			printk("i2c_outb: 0x%02x, timeout at bit #%d\n", (int)c, i);
			//bit_dbg(1, &i2c_adap->dev, "i2c_outb: 0x%02x, timeout at bit #%d\n", (int)c, i);
			return -ETIMEDOUT;
		}
		scllo(adap);
	}

	sdahi(adap);
	//gp_gpio_set_direction(hSda, GPIO_DIR_INPUT);
	//gp_gpio_set_pullfunction(hSda, GPIO_PULL_FLOATING);

	if (sclhi(adap) < 0) {
		printk("i2c_outb: 0x%02x, timeout at ack\n", (int)c);
		return -ETIMEDOUT;
	}

	ack = !getsda(adap);
	//bit_dbg(2, &i2c_adap->dev, "i2c_outb: 0x%02x %s\n", (int)c, ack ? "A" : "NA");
	printk("i2c_outb: 0x%02x %s\n", (int)c, ack ? "A" : "NA");

	scllo(adap);
	return ack;
}

static int i2c_inb(struct i2c_adapter *i2c_adap)
{
	int i;
	unsigned char indata = 0;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	sdahi(adap);
	for (i = 0; i < 8; i++) {
		if (sclhi(adap) < 0) {
			//bit_dbg(1, &i2c_adap->dev, "i2c_inb: timeout at bit #%d\n", 7 - i);
			printk("i2c_inb: timeout at bit #%d\n", 7 - i);

			return -ETIMEDOUT;
		}
		indata *= 2;
		if (getsda(adap))
			indata |= 0x01;
		setscl(adap, 0);
		udelay(i == 7 ? adap->udelay / 2 : adap->udelay);
	}
	return indata;
}

static int test_bus(struct i2c_algo_bit_data *adap, char *name)
{
	int scl, sda;

	if (adap->getscl == NULL)
		pr_info("%s: Testing SDA only, SCL is not readable\n", name);

	sda = getsda(adap);
	scl = (adap->getscl == NULL) ? 1 : getscl(adap);
	if (!scl || !sda) {
		printk("%s: bus seems to be busy\n", name);
		goto bailout;
	}

	sdalo(adap);
	sda = getsda(adap);
	scl = (adap->getscl == NULL) ? 1 : getscl(adap);
	if (sda) {

////////////////////////////////
		printk("%s: SDA stuck high!\n", name);
		goto bailout;
	}
	if (!scl) {
		printk("%s: SCL unexpected low "
		       "while pulling SDA low!\n", name);
		goto bailout;
	}

	sdahi(adap);
	sda = getsda(adap);
	scl = (adap->getscl == NULL) ? 1 : getscl(adap);
	if (!sda) {
		printk("%s: SDA stuck low!\n", name);
		goto bailout;
	}
	if (!scl) {
		printk("%s: SCL unexpected low "
		       "while pulling SDA high!\n", name);
		goto bailout;
	}

	scllo(adap);
	sda = getsda(adap);
	scl = (adap->getscl == NULL) ? 0 : getscl(adap);
	if (scl) {
		printk("%s: SCL stuck high!\n", name);
		goto bailout;
	}
	if (!sda) {
		printk("%s: SDA unexpected low "
		       "while pulling SCL low!\n", name);
		goto bailout;
	}

	sclhi(adap);
	sda = getsda(adap);
	scl = (adap->getscl == NULL) ? 1 : getscl(adap);
	if (!scl) {
		printk("%s: SCL stuck low!\n", name);
		goto bailout;
	}
	if (!sda) {
		printk("%s: SDA unexpected low "
		       "while pulling SCL high!\n", name);
		goto bailout;
	}
	pr_info("%s: Test OK\n", name);
	return 0;

bailout:
	sdahi(adap);
	sclhi(adap);
	return -ENODEV;
}

static int try_address(struct i2c_adapter *i2c_adap, unsigned char addr, int retries)
{
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;
	int i, ret = 0;

	for (i = 0; i <= retries; i++) {
		ret = i2c_outb(i2c_adap, addr);
		if (ret == 1 || i == retries)
			break;
		printk("try_address() emitting stop condition\n");
		i2c_stop(adap);
		udelay(adap->udelay);
		yield();
		printk("try_address() emitting start condition\n");
		i2c_start(adap);
	}
	if (i && ret)
		printk("Used %d tries to %s client at "
			"0x%02x: %s\n", i + 1,
			addr & 1 ? "read from" : "write to", addr >> 1,
			ret == 1 ? "success" : "failed, timeout?");
	return ret;
}

static int sendbytes(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	const unsigned char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while (count > 0) {
		retval = i2c_outb(i2c_adap, *temp);

		if ((retval > 0) || (nak_ok && (retval == 0))) {
			count--;
			temp++;
			wrcount++;
		} else if (retval == 0) {
			printk("sendbytes: NAK bailout.\n");
			//dev_err(&i2c_adap->dev, "sendbytes: NAK bailout.\n");
			return -EIO;
		} else {
			printk("sendbytes: error %d\n", retval);
			//dev_err(&i2c_adap->dev, "sendbytes: error %d\n", retval);
			return retval;
		}
	}
	return wrcount;
}

static int acknak(struct i2c_adapter *i2c_adap, int is_ack)
{
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	if (is_ack)
		setsda(adap, 0);
	udelay((adap->udelay + 1) / 2);
	if (sclhi(adap) < 0) {
		printk("readbytes: ack/nak timeout\n");
		return -ETIMEDOUT;
	}
	scllo(adap);
	return 0;
}

static int readbytes(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	int inval;
	int rdcount = 0;
	unsigned char *temp = msg->buf;
	int count = msg->len;
	const unsigned flags = msg->flags;

	while (count > 0) {
		inval = i2c_inb(i2c_adap);
		if (inval >= 0) {
			*temp = inval;
			rdcount++;
		} else {
			break;
		}

		temp++;
		count--;

		if (rdcount == 1 && (flags & I2C_M_RECV_LEN)) {
			if (inval <= 0 || inval > I2C_SMBUS_BLOCK_MAX) {
				if (!(flags & I2C_M_NO_RD_ACK))
					acknak(i2c_adap, 0);
				printk("readbytes: invalid block length (%d)\n", inval);
				return -EREMOTEIO;
			}
			count += inval;
			msg->len += inval;
		}

		printk("readbytes: 0x%02x %s\n",
			inval,
			(flags & I2C_M_NO_RD_ACK)
				? "(no ack/nak)"
				: (count ? "A" : "NA"));

		if (!(flags & I2C_M_NO_RD_ACK)) {
			inval = acknak(i2c_adap, count);
			if (inval < 0)
				return inval;
		}
	}
	return rdcount;
}

static int bit_doAddress(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	unsigned char addr;
	int ret, retries;

	retries = nak_ok ? 0 : i2c_adap->retries;

	if (flags & I2C_M_TEN) {
		addr = 0xf0 | ((msg->addr >> 7) & 0x03);
		printk("addr0: %d\n", addr);
		ret = try_address(i2c_adap, addr, retries);
		if ((ret != 1) && !nak_ok)  {
			printk("doAddress() died at extended address code\n");
			return -EREMOTEIO;
		}
		ret = i2c_outb(i2c_adap, msg->addr & 0x7f);
		if ((ret != 1) && !nak_ok) {
			printk("died at 2nd address code\n");
			return -EREMOTEIO;
		}
		if (flags & I2C_M_RD) {
			printk("doAddress() emitting repeated start condition\n");
			i2c_repstart(adap);
			addr |= 0x01;
			ret = try_address(i2c_adap, addr, retries);
			if ((ret != 1) && !nak_ok) {
				printk("doAddress() died at repeated address code\n");
				return -EREMOTEIO;
			}
		}
	} else {
		addr = msg->addr << 1;
		if (flags & I2C_M_RD)
			addr |= 1;
		if (flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;
		ret = try_address(i2c_adap, addr, retries);
		if ((ret != 1) && !nak_ok)
			return -ENXIO;
	}

	return 0;
}

static int bit_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	struct i2c_msg *pmsg;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;
	int i, ret;
	unsigned short nak_ok;

	if (adap->pre_xfer) {
		ret = adap->pre_xfer(i2c_adap);
		if (ret < 0)
			return ret;
	}

	i2c_start(adap);
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			if (i) {
				printk("bit_xfer() emitting repeated start condition\n");
				i2c_repstart(adap);
			}
			ret = bit_doAddress(i2c_adap, pmsg);
			if ((ret != 0) && !nak_ok) {
				printk("NAK from device addr 0x%02x msg #%d\n", msgs[i].addr, i);
				goto bailout;
			}
		}
		if (pmsg->flags & I2C_M_RD) {
			ret = readbytes(i2c_adap, pmsg);
			if (ret >= 1)
				printk("read %d byte%s\n", ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EREMOTEIO;
				goto bailout;
			}
		} else {
			ret = sendbytes(i2c_adap, pmsg);
			if (ret >= 1)
				printk("wrote %d byte%s\n", ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EREMOTEIO;
				goto bailout;
			}
		}
	}
	ret = i;

bailout:
	printk("bit_xfer() emitting stop condition\n");
	i2c_stop(adap);

	if (adap->post_xfer)
		adap->post_xfer(i2c_adap);
	return ret;
}

static u32 bit_func(struct i2c_adapter *adap)
{

#if 0
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
#else
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL |
	       I2C_FUNC_SMBUS_READ_BLOCK_DATA |
	       I2C_FUNC_SMBUS_BLOCK_PROC_CALL |
	       I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MANGLING;
#endif

}

struct i2c_algo_bit_data gpl8300_bit_data = {
	.data = &gpio_data,
	.setsda = gpl8300_bit_setsda,
	.setscl = gpl8300_bit_setscl,
	.getsda = gpl8300_bit_getsda,
	.getscl = gpl8300_bit_getscl,
	.udelay = DELAY,
	.timeout = 100
};

static struct i2c_algorithm _algorithm = {
	.master_xfer = bit_xfer,
	.functionality = bit_func,
};


struct i2c_adapter gpl8300_i2c_adapter = {
	.name = "GPL8300 GPIO-based audio I2C Adapter",
	.id = 1,
	.algo_data = &gpl8300_bit_data,
	.algo = &_algorithm,
	.nr = 1,
	//priv->adap.dev.parent = &pdev->dev;

	.retries = 3
};

/*
static int _probe(struct platform_device *pdev)
{
	int res = 0;

	printk(KERN_ERR "PROBE (I2C-1)\n");


	hScl = gp_gpio_request(MK_GPIO_INDEX(GPIO3_18_CHN, GPIO3_18_FUN, GPIO3_18_GID, GPIO3_18_PIN), "SCL");
	hSda = gp_gpio_request(MK_GPIO_INDEX(GPIO3_19_CHN, GPIO3_19_FUN, GPIO3_19_GID, GPIO3_19_PIN), "SDA");
	output_pin_setting();
	res = test_bus(&gpl8300_bit_data, gpl8300_i2c_adapter.name);
	if (0 != res)
	{
		printk(KERN_ERR "ERROR: I2C-1 TEST BUS FAIL \n");
		gp_gpio_release(hScl), gp_gpio_release(hSda);
		return -ENODEV;
	}

	gpl8300_i2c_adapter.dev.parent = &pdev->dev;
	if ((res = i2c_add_numbered_adapter(&gpl8300_i2c_adapter) != 0))
	{
		printk(KERN_ERR "ERROR: Could not install GPL8300 I2C-1 adapter\n");
		gp_gpio_release(hScl), gp_gpio_release(hSda);
		return res;
	}
	//platform_set_drvdata(pdev, priv);

	return res;
}

static int __devexit _remove(struct platform_device *pdev)
{
	//i2c_del_adapter(&gpl8300_i2c_adapter);
	//platform_set_drvdata(pdev, NULL);

	//gp_gpio_release(hScl), gp_gpio_release(hSda);
	return 0;
}
*/

/*
static struct platform_device _device = {
	.name = "i2c-1",
	.id = 1,
	//.dev = {
	//	.release = gp_ti2c_device_release,
	//}
};
*/

/*
static struct platform_driver _driver = {
	//.remove	 = _remove,
	//.probe	 = _probe,
	.driver	 = {
		.name = "gpl8300 audio i2c",
		.owner = THIS_MODULE,
	},
};
*/


/*

struct i2c_adapter {
	struct module *owner;
	unsigned int id;
	unsigned int class;
	const struct i2c_algorithm *algo;
	void *algo_data;

	struct rt_mutex bus_lock;

	int timeout;
	int retries;
	struct device dev;

	int nr;
	char name[48];
	struct completion dev_released;

	struct list_head userspace_clients;
};


*/

int __init gpl8300_i2c_init(void)
{
	int res;

// pin 89
	hScl = gp_gpio_request(MK_GPIO_INDEX(GPIO3_18_CHN, GPIO3_18_FUN, GPIO3_18_GID, GPIO3_18_PIN), "SCL");
printk("hScl=%p\n", hScl);
printk("hScl=%p\n", hScl);
printk("hScl=%p\n", hScl);
printk("hScl=%p\n", hScl);

	hSda = gp_gpio_request(MK_GPIO_INDEX(GPIO3_19_CHN, GPIO3_19_FUN, GPIO3_19_GID, GPIO3_19_PIN), "SDA");
printk("hSda=%p\n", hSda);
printk("hScl=%p\n", hScl);
printk("hScl=%p\n", hScl);
printk("hScl=%p\n", hScl);

	output_pin_setting();

	//gpl8300_i2c_adapter.dev.parent = &_device.dev;
	res = i2c_add_numbered_adapter(&gpl8300_i2c_adapter);

	res = test_bus(&gpl8300_bit_data, gpl8300_i2c_adapter.name);
	if (0 != res)		printk("I2C-1 TEST BUS FAIL \n");
	else				printk("I2C-1 TEST OK \n");


	//platform_device_register(&_device);
	//platform_driver_register(&_driver);


/*
	//gpl8300_i2c_adapter.dev.parent = &_driver.driver;

	res = test_bus(&gpl8300_bit_data, gpl8300_i2c_adapter.name);
	if (0 != res)
	{

		gp_gpio_release(hScl), gp_gpio_release(hSda);
		return -ENODEV;
	}

	// i2c_bit_add_numbered_bus
	//if ((res = i2c_bit_add_bus(&gpl8300_i2c2_adapter) != 0))
	if ((res = i2c_add_numbered_adapter(&gpl8300_i2c_adapter) != 0))
	{
		printk(KERN_ERR "ERROR: Could not install GPL8300 I2C-1 adapter\n");
		return res;
	}
*/
	return 0;

}

void __exit gpl8300_i2c_exit(void)
{
	i2c_del_adapter(&gpl8300_i2c_adapter);
	//platform_set_drvdata(pdev, NULL);
	gp_gpio_release(hScl), gp_gpio_release(hSda);

	//platform_device_unregister(&_device);
	//platform_driver_unregister(&_driver);
}

//subsys_initcall(gpl8300_i2c_init);
module_init(gpl8300_i2c_init);
module_exit(gpl8300_i2c_exit);

MODULE_DESCRIPTION("GPIO-based I2C adapter for GPL8300 systems");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Namo<namo@pentamicro.com>");

