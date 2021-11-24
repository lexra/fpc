#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-id.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/au1000.h>

#if 0
/* GPIO2, Au1500 only */
#define GPIO2_BASE			0xB1700000
#define GPIO2_DIR			(GPIO2_BASE + 0)
#define GPIO2_OUTPUT			(GPIO2_BASE + 8)
#define GPIO2_PINSTATE		(GPIO2_BASE + 0xC)
#define GPIO2_INTENABLE		(GPIO2_BASE + 0x10)
#define GPIO2_ENABLE			(GPIO2_BASE + 0x14)
#endif 

#define I2C0_BIT_GPDATA	0x00000002
#define I2C0_BIT_GPCLK		0x00000004
#define I2C1_BIT_GPDATA	0x00000008
#define I2C1_BIT_GPCLK		0x00000010
#define I2C2_BIT_GPDATA	0x00000020
#define I2C2_BIT_GPCLK		0x00000040

static struct au1x00_i2c_data {
	unsigned int sda_line;
	unsigned int scl_line;
} gpio_data;



static void au1x00_bit_setscl(void *data, int val)
{
	unsigned short value, temp;

	value = au_readw(GPIO2_DIR);
	temp = value & I2C0_BIT_GPCLK;
	if(temp == 0)
	{
		value |= I2C0_BIT_GPCLK;
		au_writew(value, GPIO2_DIR);		/* set GPIO202 output */
	}

	value = au_readw(GPIO2_OUTPUT + 2);	/* ENA field of Data Output Register */
	value |= (I2C0_BIT_GPCLK);
	au_writew(value, GPIO2_OUTPUT + 2);	/* enable Data Output Register */

	if(val != 0)
	{
		value = au_readw(GPIO2_OUTPUT);
		value |= (I2C0_BIT_GPCLK);
		au_writew(value, GPIO2_OUTPUT);	/* set DATA field of Data Output Register for GPIO202 Pin */
	}
	else	
	{
		value = au_readw(GPIO2_OUTPUT);
		value &= (~I2C0_BIT_GPCLK);
		au_writew(value, GPIO2_OUTPUT);	/* claer DATA field of Data Output Register for GPIO202 Pin */
	}
}

static void au1x00_bit_setscl2(void *data, int val)
{

#if 1
	unsigned short value, temp;

	value = au_readw(GPIO2_DIR);
	temp = value & I2C1_BIT_GPCLK;
	if(temp == 0)							/* if GPIO204 pin is input */
	{
		value |= I2C1_BIT_GPCLK;
		au_writew(value, GPIO2_DIR);		/* set GPIO204 output */
	}

	value = au_readw(GPIO2_OUTPUT + 2);
	value |= (I2C1_BIT_GPCLK);
	au_writew(value, GPIO2_OUTPUT + 2);	/* GPIO204 Data output is enable */

	if(val != 0)
	{
		value = au_readw(GPIO2_OUTPUT);
		value |= (I2C1_BIT_GPCLK);
		au_writew(value, GPIO2_OUTPUT);	/* set GPIO204 Output Data bit */
	}
	else
	{
		value = au_readw(GPIO2_OUTPUT);
		value &= (~I2C1_BIT_GPCLK);
		au_writew(value, GPIO2_OUTPUT);	/* clear GPIO204 Output Data bit */
	}
#else
	if (val)
		*gpio_data_en |= (I2C1_BIT_GPCLK | I2C1_BIT_GPCLK << 16);
	else	
		*gpio_data_en = (I2C1_BIT_GPCLK << 16);
#endif

}

static void au1x00_bit_setsda(void *data, int val)
{
	unsigned short value, temp;

	/* Set Data pin is an Output */
	value = au_readw(GPIO2_DIR);
	temp = value & I2C0_BIT_GPDATA;
	if(temp == 0)
	{
		value |= I2C0_BIT_GPDATA;
		au_writew(value, GPIO2_DIR);
	}

	/* Set Data pin's Data Output is enabled */
	value = au_readw(GPIO2_OUTPUT + 2);
	value |= (I2C0_BIT_GPDATA);
	au_writew(value, GPIO2_OUTPUT + 2);

	if(val != 0)
	{
		/* Set Data pin's Output Data */
		value = au_readw(GPIO2_OUTPUT);
		value |= (I2C0_BIT_GPDATA);
		au_writew(value, GPIO2_OUTPUT);
	}
	else
	{
		/* Clear Data pin's Output Data */
		value = au_readw(GPIO2_OUTPUT);
		value &= (~I2C0_BIT_GPDATA);
		au_writew(value, GPIO2_OUTPUT);
	}
}

static void au1x00_bit_setsda2(void *data, int val)
{
	unsigned short value, temp;
	
	value = au_readw(GPIO2_DIR);
	temp = value & I2C1_BIT_GPDATA;
	if(temp == 0)
	{
		value |= I2C1_BIT_GPDATA;
		au_writew(value, GPIO2_DIR);
	}

	value = au_readw(GPIO2_OUTPUT + 2);
	value |= (I2C1_BIT_GPDATA);
	au_writew(value, GPIO2_OUTPUT + 2);

	if(val != 0)
	{
		value = au_readw(GPIO2_OUTPUT);
		value |= (I2C1_BIT_GPDATA);
		au_writew(value, GPIO2_OUTPUT);
	}
	else
	{
		value = au_readw(GPIO2_OUTPUT);
		value &= (~I2C1_BIT_GPDATA);
		au_writew(value, GPIO2_OUTPUT);
	}
}

static int au1x00_bit_getscl(void *data)
{
	int val;
	unsigned short value, temp;

	value = au_readw(GPIO2_DIR);
	value &= ~I2C0_BIT_GPCLK;
	au_writew(value, GPIO2_DIR);

	value = au_readw(GPIO2_OUTPUT + 2);
	value &= ~I2C0_BIT_GPCLK;
	au_writew(value, GPIO2_OUTPUT + 2);

	value = au_readw(GPIO2_PINSTATE);
	value &= I2C0_BIT_GPCLK;
	if(value == I2C0_BIT_GPCLK)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}

	value = au_readw(GPIO2_DIR);
	value |= I2C0_BIT_GPCLK;
	au_writew(value, GPIO2_DIR);

	value = au_readw(GPIO2_OUTPUT + 2);
	value |= I2C0_BIT_GPCLK;
	au_writew(value, GPIO2_OUTPUT + 2);

	return val;
}	

static int au1x00_bit_getscl2(void *data)
{
	int val;
	unsigned short value, temp;

	value = au_readw(GPIO2_DIR);
	value &= ~I2C1_BIT_GPCLK;
	au_writew(value, GPIO2_DIR);

	value = au_readw(GPIO2_OUTPUT + 2);
	value &= ~I2C1_BIT_GPCLK;
	au_writew(value, GPIO2_OUTPUT + 2);

	value = au_readw(GPIO2_PINSTATE);
	value &= I2C1_BIT_GPCLK;
	if(value == I2C1_BIT_GPCLK)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}

	value = au_readw(GPIO2_DIR);
	value |= I2C1_BIT_GPCLK;
	au_writew(value, GPIO2_DIR);

	value = au_readw(GPIO2_OUTPUT + 2);
	value |= I2C1_BIT_GPCLK;
	au_writew(value, GPIO2_OUTPUT + 2);

	return val;
}

static int au1x00_bit_getsda(void *data)
{

#if 1
	int val;
	unsigned short value, temp;

	value = au_readw(GPIO2_PINSTATE);
	value &= I2C0_BIT_GPDATA;
	if(value == I2C0_BIT_GPDATA)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}

	return val;
#else
	return 0;
#endif	

}	

static int au1x00_bit_getsda2(void *data)
{

#if 1
	int val;
	unsigned short value, temp;

	value = au_readw(GPIO2_PINSTATE);
	value &= I2C1_BIT_GPDATA;

	if(value == I2C1_BIT_GPDATA)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}

	return val;
#else
	return 0;
#endif	

}
static void au1x00_i2c_inc_use(struct i2c_adapter *adap)
{
	MOD_INC_USE_COUNT;
}

static void au1x00_i2c_dec_use(struct i2c_adapter *adap)
{
	MOD_DEC_USE_COUNT;
}

struct i2c_algo_bit_data au1x00_bit_data = {
	.data = &gpio_data,
	.setsda = au1x00_bit_setsda,
	.setscl = au1x00_bit_setscl,
	.getsda = au1x00_bit_getsda,
	.getscl = au1x00_bit_getscl,

#ifdef OURBOARD
	.udelay = 10,
#else
	.udelay = 10,
#endif

	.mdelay = 10,
	.timeout	= 100
};

struct i2c_adapter au1x00_i2c_adapter = {
	.name = "AU1x00 I2C-0 Adapter",
	.id = I2C_HW_B_AU1x00,
	.algo_data = &au1x00_bit_data,
	.inc_use = au1x00_i2c_inc_use,
	.dec_use = au1x00_i2c_dec_use,
	.nr = 0,
};


struct i2c_algo_bit_data au1x00_bit_data2 = {
	.data = &gpio_data,
	.setsda = au1x00_bit_setsda2,
	.setscl = au1x00_bit_setscl2,
	.getsda = au1x00_bit_getsda2,
	.getscl = au1x00_bit_getscl2,

#ifdef OURBOARD
	.udelay = 10,
#else
	.udelay = 10,
#endif

	.mdelay = 10,
	.timeout = 100
};

struct i2c_adapter au1x00_i2c2_adapter = {
	.name = "AU1x00 I2C-1 Adapter",
	.id = I2C_HW_B_AU1x00,
	.algo_data = &au1x00_bit_data2,
	.inc_use = au1x00_i2c_inc_use,
	.dec_use = au1x00_i2c_dec_use,
	.nr = 1,
};

int __init au1x00_i2c_init(void)
{
	int res;
	unsigned short value;
	
	printk("AU1x00 GPIO2 based I2C0/1 driver init\n");

	output_pin_setting();

#if 1
	if ((res = i2c_bit_add_bus(&au1x00_i2c_adapter) != 0))
	{
		printk(KERN_ERR "ERROR: Could not install AU1x00 I2C-0 adapter\n");
		return res;
	}
#endif

	if ((res = i2c_bit_add_bus(&au1x00_i2c2_adapter) != 0))
	{
		printk(KERN_ERR "ERROR: Could not install AU1x00 I2C-1 adapter\n");
		return res;
	}

	return 0;
}

void __exit au1x00_i2c_exit(void)
{
	i2c_bit_del_bus(&au1x00_i2c_adapter);
}

void input_pin_setting(void)
{
	unsigned short value, temp;
	
	value = au_readw(GPIO2_DIR);
	value &= ~(I2C0_BIT_GPDATA | I2C1_BIT_GPDATA);
	au_writew(value, GPIO2_DIR);					/* set GPIO201/203 pin as input pin */

	value = au_readw(GPIO2_OUTPUT + 2);
	value &= ~(I2C0_BIT_GPDATA | I2C1_BIT_GPDATA);
	au_writew(value, GPIO2_OUTPUT + 2);			/* set GPIO201/203 outpin pin disabled */
}

void output_pin_setting(void)
{
	unsigned short value, temp;
	
	value = au_readw(GPIO2_DIR);
	value |= (I2C0_BIT_GPDATA | I2C1_BIT_GPDATA);
	au_writew(value, GPIO2_DIR);					/* set GPIO201/203 pin as output pin */
	
	value = au_readw(GPIO2_OUTPUT + 2);			/* Data Output Register ENA field */
	value |= (I2C0_BIT_GPDATA | I2C1_BIT_GPDATA);
	au_writew(value, GPIO2_OUTPUT + 2);			/* set GPIO201/203 outpin pin enabled */
}

module_init(au1x00_i2c_init);
module_exit(au1x00_i2c_exit);

MODULE_DESCRIPTION("GPIO-based I2C adapter for AU1x00 systems");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Namo<namo@pentamicro.com>");

