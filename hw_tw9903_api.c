/* set tabstop=4 */
/********************************************************************************
 *                                                                              *
 * Copyright(C) 2004  Penta-Micro                                               *
 *                                                                              *
 * ALL RIGHT RESERVED                                                           *
 *                                                                              *
 * This software is the property of Penta-Micro and is furnished under          *
 * license by Penta-Micro. This software may be used only in accordance         *	
 * with the terms of said license. This copyright notice may not be             *
 * removed, modified or obliterated without the prior written permission        *
 * of Penta-Micro.                                                              *
 *                                                                              *
 * This software may not be copyed, transmitted, provided to or otherwise       *
 * made available to any other person, company, corporation	or other entity     *
 * except as specified in the terms of said license.                            *
 *                                                                              *
 * No right, title, ownership or other interest in the software is hereby       *
 * granted or transferred.                                                      *
 *                                                                              *
 * The information contained herein is subject to change without notice and     *
 * should not be construed as a commitment by Penta-Micro.                      *
 *                                                                              *
 ********************************************************************************
 
  MODULE NAME:  HW_TW9903_API.C
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
  06/28/2004 2.0 CheulBeck(whitefe)       Created   
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains functions for TW9903.
  
 ...............................................................................
*/  
 

/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "hw_tw9903_api.h"


/** ************************************************************************* ** 
 ** locals
 ** ************************************************************************* **/
static S32 fd_i2c0 = -1;


int tw99_open()
{
	if (fd_i2c0 >= 0)
		return fd_i2c0;

	fd_i2c0 = open("/dev/i2c-0", O_RDWR);
	if (fd_i2c0 < 0)
	{
		fd_i2c0 = -1;

		printf("[at2041_api] tw99 i2c driver open failed\n");
		return -1;
	}

	return fd_i2c0;
}

void tw99_close()
{
	close(fd_i2c0);
	fd_i2c0 = -1;
}

int tw99_write_data(int slave_adr, int sub_adr, char *data, int length)
{
	int i, res;
	char buf[256];	

	res = ioctl(fd_i2c0, I2C_SLAVE_FORCE, slave_adr);
	if(res < 0)
	{
		fprintf(stderr, "I2C: set slave addr error\n");
		return -1;
	}

	buf[0] = sub_adr;
	for(i = 1; i <= length; i++)
	{
		buf[i] = data[i -1];
	}

	if(write(fd_i2c0, buf, length + 1) != length + 1)
	{
		fprintf(stderr, "I2C: write data error %x\n", sub_adr);
		return -1;
	}

	return 0;
}

int tw99_read_data(int slave_adr, int sub_adr, unsigned char *data, int length)
{
	int i, res;

	res = ioctl(fd_i2c0, I2C_SLAVE_FORCE, slave_adr);
	if(res < 0)
	{
		fprintf(stderr, "I2C: set slave addr error\n");
		return -1;
	}

	if(write(fd_i2c0, &sub_adr, 1) != 1)
	{
		fprintf(stderr, "I2C: write data error %x\n", sub_adr);
		return -1;
	}

	return read(fd_i2c0, data, length);
}


