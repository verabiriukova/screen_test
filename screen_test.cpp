/*
 * AD7705 test program for the Raspberry PI
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 * Copyright (c) 2013  Bernd Porr <mail@berndporr.me.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "gpio-sysfs.h" //need this ?


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) //need this ?

static void pabort(const char *s) //error printing
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode = SPI_CPHA;
static uint8_t bits = 8;
static uint32_t speed = 40000;
static uint16_t delay = 10;
static int drdy_GPIO = 22; //need this ?



tatic void readData(int fd)
{
  int ret;
  uint8_t tx1[5] = {0};
  uint8_t rx1[5] = {0};
	int i;
  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)tx1;
  tr.rx_buf = (unsigned long)rx1;
  tr.len = ARRAY_SIZE(tx1);
  tr.delay_usecs = delay;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;
  
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
    pabort("can't send spi message");
	for(i=0; i<5; i++){
		printf("byte %d = %d \n\r", i, rx1[i])	
	}
}




int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	int sysfs_fd;

	int no_tty = !isatty( fileno(stdout) );

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	fprintf(stderr, "spi mode: %d\n", mode);
	fprintf(stderr, "bits per word: %d\n", bits);
	fprintf(stderr, "max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	// enable master clock for the AD
	// divisor results in roughly 4.9MHz
	// this also inits the general purpose IO
	////////gz_clock_ena(GZ_CLK_5MHz,5);

	// enables sysfs entry for the GPIO pin
	////////gpio_export(drdy_GPIO);
	// set to input
	////////gpio_set_dir(drdy_GPIO,0);
	// set interrupt detection to falling edge
	////////gpio_set_edge(drdy_GPIO,"falling");
	// get a file descriptor for the GPIO pin
	////////sysfs_fd = gpio_fd_open(drdy_GPIO);

	// resets the AD7705 so that it expects a write to the communication register
	////////writeReset(fd);

	// tell the AD7705 that the next write will be to the clock register
	////////writeReg(fd,0x20);
	// write 00001100 : CLOCKDIV=1,CLK=1,expects 4.9152MHz input clock
	////////writeReg(fd,0x0C);

	// tell the AD7705 that the next write will be the setup register
	////////writeReg(fd,0x10);
	// intiates a self calibration and then after that starts converting
	////////writeReg(fd,0x40);

	// we read data in an endless loop and display it
	// this needs to run in a thread ideally
	while (1) {

	  // let's wait for data for max one second
	/*  ret = gpio_poll(sysfs_fd,1000);
	  if (ret<1) {
	    fprintf(stderr,"Poll error %d\n",ret);
	  }*/

	  // tell the AD7705 to read the data register (16 bits)
	 //////// writeReg(fd,0x38);
	  // read the data register by performing two 8 bit reads
	  int value = readData(fd);
		fprintf(stderr,"data = %d       \r",value);

		// if stdout is redirected to a file or pipe, output the data
		if( no_tty )
		{
			printf("%d\n", value);
			fflush(stdout);
		}
	}

	close(fd);
	gpio_fd_close(sysfs_fd);

	return ret;
}
