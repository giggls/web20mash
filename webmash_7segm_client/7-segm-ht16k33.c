/* 

IO-functions for adafruit HT16K33 7-segment display

(c) 2013 Sven Geggus <sven-web20mash@geggus.net>

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "cmdline.h"

extern Cmdline *cmd;

extern void die(char* fmt, ...);


void i2c_write_or_die(int fd, uint8_t cmd, uint8_t value) {
  if (-1==i2c_smbus_write_byte_data(fd,cmd,value))
    die("error in i2c_smbus_write_byte_data(0x%02x,0x%02x)\n",cmd,value);
}

// generate led codes from asci value
uint8_t ascii2led(uint8_t val) {
  uint8_t ledvals[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
  if ((val>0x2f) && (val <0x3a)) return ledvals[val-0x30];
  if (val==0x2d) return 0x40;
  return 0;
}

int init_ht16k33_7segm_display(char *device, int i2caddr) {
  int i2cfd;

  i2cfd=open(device, O_RDWR);
  if (i2cfd < 0)
    die("unable to open i2c bus: %s\n",device);
  if (ioctl(i2cfd, I2C_SLAVE, i2caddr) < 0)
    die("unable to open i2c device at address 0x%x\n",i2caddr);
  // initialize display
  i2c_write_or_die(i2cfd,0x21,0x00);
  i2c_write_or_die(i2cfd,0x81,0x00);
  i2c_write_or_die(i2cfd,0xef,0x00);
  return i2cfd;
}


void update_ht16k33_7segm_display(char *value,bool must, int i2cfd) {
  static char oldval[]="888.8";
  uint8_t digits[]={8,6,2,0};
  char dispval[6];
  int i,dotpos;
  char *dotaddr;
  int digit;

  if (strcmp(oldval,value)!=0) {
    strncpy(oldval,value,6);
    oldval[6]='\0';
    // update display digit by digit
    snprintf(dispval,6,"%5s",value);
    dispval[5]='\0';

    // search position of decimal point
    dotaddr=strchr(value,'.');
    if (NULL!=dotaddr) {
      dotpos=strlen(value)-1-(dotaddr-&value[0]);
    } else {
      dotpos=-1;
    }

    digit=0;
    for (i=4;digit<4;i--) {
      uint8_t dval;

      // ignore the dot
      if (dispval[i]=='.') continue;

      // activate decimal point if required
      if (dotpos==digit) {
	dval=ascii2led(dispval[i])+0x80;
      } else {
	dval=ascii2led(dispval[i]);
	if ((3==digit) && (must))
	  dval=0x48;
      }
      i2c_write_or_die(i2cfd,digits[digit],dval);
      
      digit++;
    }

  }
}
