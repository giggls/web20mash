/*
 *
 * Plugin for TEMPer1 USB temperature sensor from http://pcsensor.com/
 *
 * sensor_temper1.so Sven Geggus (c) 2014-2026 (sven-web20mash@geggus.net)
 * based on pcsensor.c by Philipp Adelt (c) 2012 (info@philipp.adelt.net)
 * based on Juan Carlos Perez (c) 2011 (cray@isp-sl.com)
 * based on Temper.c by Robert Kavaler (c) 2009 (relavak.com)
 * All rights reserved.
 *
 * Temper driver for linux. This program can be compiled either as a library
 * or as a standalone program (-DUNIT_TEST). The driver will work with some
 * TEMPer usb devices from RDing (www.PCsensor.com).
 *
 * This driver works with USB devices presenting ID 0c45:7401.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Philipp Adelt (and other contributors) ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Philipp Adelt (or other contributors) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <time.h>

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <stdbool.h>

#include "minIni.h"
#include "cfgdflt.h"
#include "errorcodes.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

static char plugin_name[]="sensor_temper1";

// default is no error
static int plugin_error=0;

static int device;

extern bool sensor_simul;
extern void debug(char* fmt, ...);
extern void errorlog(char* fmt, ...);
extern void die(char* fmt, ...);

#define PREFIX "[TEMPer1 sensor plugin] "

#define VERSION "1.0.0"

#define VENDOR_ID  0x0c45
#define PRODUCT_ID 0x7401

#define INTERFACE1 0x00
#define INTERFACE2 0x01

static const int reqIntLen=8;
static const int timeout=5000; /* timeout in ms */

static const unsigned char uTemperatura[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char uIni1[] = { 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char uIni2[] = { 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };

static int udebug=0;
static int calibration=0;

static libusb_context *ctx = NULL;
static libusb_device_handle *lvr_winusb = NULL;

static libusb_device_handle *find_lvr_winusb(int devicenum);

static void usb_detach(libusb_device_handle *lvr_winusb, int iInterface) {
        int ret;

        ret = libusb_detach_kernel_driver(lvr_winusb, iInterface);
        if(ret) {
                if(ret == LIBUSB_ERROR_NOT_FOUND) {
                        if(udebug) {
                                debug(PREFIX"Device already detached\n");
                        }
                } else {
                        if(udebug) {
                                debug(PREFIX"Detach failed: %s[%d]\n",
                                       libusb_strerror(ret), ret);
                                debug(PREFIX"Continuing anyway\n");
                        }
                }
        } else {
                if(udebug) {
                        debug(PREFIX"detach successful\n");
                }
        }
}

static libusb_device_handle* setup_libusb_access(int devicenum) {
     libusb_device_handle *lvr_winusb;
     int r;

     r = libusb_init(&ctx);
     if (r < 0) {
             debug(PREFIX"Failed to initialize libusb: %s\n", libusb_strerror(r));
             return NULL;
     }

     if(udebug) {
#if LIBUSB_API_VERSION >= 0x01000106
        libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
#else
        libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);
#endif
     } else {
#if LIBUSB_API_VERSION >= 0x01000106
        libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
#else
        libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_NONE);
#endif
     }

     if(!(lvr_winusb = find_lvr_winusb(devicenum))) {
                debug(PREFIX"Couldn't find the USB device, Exiting\n");
                return NULL;
        }


        usb_detach(lvr_winusb, INTERFACE1);


        usb_detach(lvr_winusb, INTERFACE2);


        if (libusb_set_configuration(lvr_winusb, 0x01) < 0) {
                debug(PREFIX"Could not set configuration 1\n");
                return NULL;
        }


        // Microdia tiene 2 interfaces
        if (libusb_claim_interface(lvr_winusb, INTERFACE1) < 0) {
                debug(PREFIX"Could not claim interface\n");
                return NULL;
        }

        if (libusb_claim_interface(lvr_winusb, INTERFACE2) < 0) {
                debug(PREFIX"Could not claim interface\n");
                return NULL;
        }

        return lvr_winusb;
}



static libusb_device_handle *find_lvr_winusb(int devicenum) {
        // iterates to the devicenum'th device for installations with multiple sensors
        libusb_device **devs;
        libusb_device *dev;
        ssize_t cnt;
        int i = 0;

        cnt = libusb_get_device_list(ctx, &devs);
        if (cnt < 0) {
                debug(PREFIX"Failed to get device list\n");
                return NULL;
        }

        while ((dev = devs[i++]) != NULL) {
                struct libusb_device_descriptor desc;
                int r = libusb_get_device_descriptor(dev, &desc);
                if (r < 0) {
                        continue;
                }

                if (desc.idVendor == VENDOR_ID &&
                    desc.idProduct == PRODUCT_ID) {
                        if (devicenum > 0) {
                                devicenum--;
                                continue;
                        }

                        libusb_device_handle *handle;
                        if(udebug) {
                                debug(PREFIX"Bus %03d Device %03d: found device with ID %04x:%04x\n",
                                       libusb_get_bus_number(dev),
                                       libusb_get_device_address(dev),
                                       desc.idVendor, desc.idProduct);
                        }

                        r = libusb_open(dev, &handle);
                        if (r < 0) {
                                debug(PREFIX"Could not open USB device: %s\n",
                                       libusb_strerror(r));
                                libusb_free_device_list(devs, 1);
                                return NULL;
                        }
                        libusb_free_device_list(devs, 1);
                        return handle;
                }
        }
        libusb_free_device_list(devs, 1);
        return NULL;
}


static void ini_control_transfer(libusb_device_handle *dev) {
    int r,i;

    unsigned char question[] = { 0x01,0x01 };

    r = libusb_control_transfer(dev, 0x21, 0x09, 0x0201, 0x00,
                                question, sizeof(question), timeout);
    if( r < 0 )
    {
          errorlog(PREFIX"USB control write: %s", libusb_strerror(r));
          die(PREFIX"USB write failed");
    }


    if(udebug) {
      for (i=0;i<reqIntLen; i++) debug(PREFIX"%02x ",question[i] & 0xFF);
      debug(PREFIX"\n");
    }
}

static void control_transfer(libusb_device_handle *dev, const unsigned char *pquestion) {
    int r,i;

    unsigned char question[reqIntLen];

    memcpy(question, pquestion, sizeof question);

    r = libusb_control_transfer(dev, 0x21, 0x09, 0x0200, 0x01,
                                question, reqIntLen, timeout);
    if( r < 0 )
    {
          errorlog(PREFIX"USB control write: %s", libusb_strerror(r));
          die(PREFIX"USB write failed");
    }

    if(udebug) {

        for (i=0;i<reqIntLen; i++) fprintf(stderr,"%02x ",question[i]  & 0xFF);
        fprintf(stderr,"\n");
    }
}

static void interrupt_read(libusb_device_handle *dev) {

    int r,i;
    int actual_length;
    unsigned char answer[reqIntLen];
    memset(answer, 0, reqIntLen);

    r = libusb_interrupt_transfer(dev, 0x82, answer, reqIntLen,
                                  &actual_length, timeout);
    if( r < 0 || actual_length != reqIntLen )
    {
          errorlog(PREFIX"USB interrupt read: %s", libusb_strerror(r));
          die(PREFIX"USB read failed");
    }

    if(udebug) {
       for (i=0;i<reqIntLen; i++) fprintf(stderr,"%02x ",answer[i]  & 0xFF);

       fprintf(stderr,"\n");
    }
}

static void interrupt_read_temperatura(libusb_device_handle *dev, float *tempC) {

    int r,i, temperature;
    int actual_length;
    unsigned char answer[reqIntLen];
    memset(answer, 0, reqIntLen);

    r = libusb_interrupt_transfer(dev, 0x82, answer, reqIntLen,
                                  &actual_length, timeout);
    if( r < 0 || actual_length != reqIntLen )
    {
          errorlog(PREFIX"USB interrupt read: %s", libusb_strerror(r));
          die(PREFIX"USB read failed");
    }


    if(udebug) {
      for (i=0;i<reqIntLen; i++) fprintf(stderr,"%02x ",answer[i]  & 0xFF);

      fprintf(stderr,"\n");
    }

    temperature = (answer[3] & 0xFF) + (answer[2] << 8);
    temperature += calibration;
    *tempC = temperature * (125.0 / 32000.0);

}

static void querySensorList(int max, char *list) {
  libusb_device **devs;
  libusb_device *dev;
  ssize_t cnt;
  int idx = 0;
  int devnum;
  size_t olen1,olen2,opos;
  bool overflow;

  cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    list[0]='\0';
    return;
  }

  devnum=0;
  overflow=false;
  list[0]='\0';
  olen2=max;
  opos=0;

  while ((dev = devs[idx++]) != NULL) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) continue;

    if (desc.idVendor == VENDOR_ID &&
        desc.idProduct == PRODUCT_ID ) {
      uint8_t busnum = libusb_get_bus_number(dev);
      uint8_t devaddr = libusb_get_device_address(dev);
      debug(PREFIX"Bus %03d Device %03d ID %04x:%04x: %d\n",
         busnum, devaddr,
         desc.idVendor, desc.idProduct, devnum);
      if (!overflow) {
        olen1=snprintf(list+opos,olen2,"\"Bus %03d Device %03d ID %04x:%04x: %d\",",
                       busnum, devaddr,
                       desc.idVendor, desc.idProduct, devnum);
        if (olen1>=olen2) {
          overflow=true;
        }
        opos+=olen1;
        olen2-=olen1;
      }
      devnum++;
    }
  }
  libusb_free_device_list(devs, 1);

  if (!overflow && opos > 0) {
    list[opos-1]='\0';
  }
}

float sensor_getTemp() {
  float temperature;
  control_transfer(lvr_winusb, uTemperatura );
  interrupt_read_temperatura(lvr_winusb, &temperature);
  debug(PREFIX"sensor_getTemp %f\n",temperature);

  return temperature;
}

void sensor_initfunc(char *cfgfile) {

  debug(PREFIX"sensor_initfunc called\n");
  // read actuator specific configuration options
  device=ini_getl("sensor_plugin_temper1","sensor",0, cfgfile);

  if ((lvr_winusb = setup_libusb_access(device)) == NULL) {
    errorlog(PREFIX"OK unable to access sensor %d:\n",device);
    errorlog(PREFIX"falling back to simulation mode\n");
    sensor_simul=true;
    plugin_error=ERROR_DEV_NOTFOUND;
    return;
  }
  ini_control_transfer(lvr_winusb);

  control_transfer(lvr_winusb, uTemperatura );
  interrupt_read(lvr_winusb);

  control_transfer(lvr_winusb, uIni1 );
  interrupt_read(lvr_winusb);

  control_transfer(lvr_winusb, uIni2 );
  interrupt_read(lvr_winusb);
  interrupt_read(lvr_winusb);
  debug(PREFIX"OK using sensor %d...\n",device);
}

size_t sensor_getInfo(size_t max, char *buf) {
  size_t pos, rest;
  char devlist[1024];

  debug(PREFIX"sensor_getInfo called\n");

  querySensorList(1024,devlist);

  pos=snprintf(buf,max,
  "  {\n"\
  "    \"type\": \"sensor\",\n"\
  "    \"name\": \"%s\",\n"\
  "    \"device\": \"%d\",\n"
  "    \"error\": \"%d\",\n",
  plugin_name, device, plugin_error);
  if (pos >=max) {
    buf[0]='\0';
    return(0);
  }
  rest=max-pos;
  pos+=snprintf(buf+pos,rest,
  "    \"devlist\": [%s]\n",devlist);
  if (pos >=max) {
    buf[0]='\0';
    return(0);
  }
  rest=max-pos;
  pos+=snprintf(buf+pos,rest,
  "  }\n");
  if (pos >=max) {
    buf[0]='\0';
  }
  return(strlen(buf));
}