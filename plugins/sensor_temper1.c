/*
 *
 * Plugin for TEMPer1 USb temperature sensor from http://pcsensor.com/
 *
 * sensor_temper1.so Sven Geggus (c) 2014 (sven-web20mash@geggus.net)
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
#include <usb.h>
#include <stdio.h>
#include <time.h>

#include <string.h>
#include <errno.h>
#include <signal.h> 
#include <syslog.h>

#include "minIni.h"
#include "cfgdflt.h"
#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))
extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
 
#define VERSION "1.0.0"
 
#define VENDOR_ID  0x0c45
#define PRODUCT_ID 0x7401
 
#define INTERFACE1 0x00
#define INTERFACE2 0x01
 
static const int reqIntLen=8;
static const int reqBulkLen=8;
static const int endpoint_Int_in=0x82; /* endpoint 0x81 address for IN */
static const int endpoint_Int_out=0x00; /* endpoint 1 address for OUT */
static const int endpoint_Bulk_in=0x82; /* endpoint 0x81 address for IN */
static const int endpoint_Bulk_out=0x00; /* endpoint 1 address for OUT */
static const int timeout=5000; /* timeout in ms */
 
static const char uTemperatura[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const char uIni1[] = { 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const char uIni2[] = { 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };

static int udebug=0;
static int calibration=0;

static usb_dev_handle *lvr_winusb = NULL;

void bad(const char *why) {
        fprintf(stderr,"Fatal error> %s\n",why);
        exit(17);
}
 
 
usb_dev_handle *find_lvr_winusb();
 
void usb_detach(usb_dev_handle *lvr_winusb, int iInterface) {
        int ret;
 
	ret = usb_detach_kernel_driver_np(lvr_winusb, iInterface);
	if(ret) {
		if(errno == ENODATA) {
			if(udebug) {
				printf("Device already detached\n");
			}
		} else {
			if(udebug) {
				printf("Detach failed: %s[%d]\n",
				       strerror(errno), errno);
				printf("Continuing anyway\n");
			}
		}
	} else {
		if(udebug) {
			printf("detach successful\n");
		}
	}
} 

usb_dev_handle* setup_libusb_access(int devicenum) {
     usb_dev_handle *lvr_winusb;

     if(udebug) {
        usb_set_debug(255);
     } else {
        usb_set_debug(0);
     }
     usb_init();
     usb_find_busses();
     usb_find_devices();
             
 
     if(!(lvr_winusb = find_lvr_winusb(devicenum))) {
                printf("Couldn't find the USB device, Exiting\n");
                return NULL;
        }
        
        
        usb_detach(lvr_winusb, INTERFACE1);
        

        usb_detach(lvr_winusb, INTERFACE2);
        
 
        if (usb_set_configuration(lvr_winusb, 0x01) < 0) {
                printf("Could not set configuration 1\n");
                return NULL;
        }
 

        // Microdia tiene 2 interfaces
        if (usb_claim_interface(lvr_winusb, INTERFACE1) < 0) {
                printf("Could not claim interface\n");
                return NULL;
        }
 
        if (usb_claim_interface(lvr_winusb, INTERFACE2) < 0) {
                printf("Could not claim interface\n");
                return NULL;
        }
 
        return lvr_winusb;
}
 
 
 
usb_dev_handle *find_lvr_winusb(int devicenum) {
        // iterates to the devicenum'th device for installations with multiple sensors
        struct usb_bus *bus;
        struct usb_device *dev;
 
        for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
                        if (dev->descriptor.idVendor == VENDOR_ID && 
                                dev->descriptor.idProduct == PRODUCT_ID ) {
                                if (devicenum>0) {
                                  devicenum--;
                                  continue;
                                }
                                usb_dev_handle *handle;
                                if(udebug) {
                                  printf("lvr_winusb with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
                                }
 
                                if (!(handle = usb_open(dev))) {
                                        printf("Could not open USB device\n");
                                        return NULL;
                                }
                                return handle;
                        }
                }
        }
        return NULL;
}
 
 
void ini_control_transfer(usb_dev_handle *dev) {
    int r,i;

    char question[] = { 0x01,0x01 };

    r = usb_control_msg(dev, 0x21, 0x09, 0x0201, 0x00, (char *) question, 2, timeout);
    if( r < 0 )
    {
          perror("USB control write"); bad("USB write failed"); 
    }


    if(udebug) {
      for (i=0;i<reqIntLen; i++) printf("%02x ",question[i] & 0xFF);
      printf("\n");
    }
}
 
void control_transfer(usb_dev_handle *dev, const char *pquestion) {
    int r,i;

    char question[reqIntLen];
    
    memcpy(question, pquestion, sizeof question);

    r = usb_control_msg(dev, 0x21, 0x09, 0x0200, 0x01, (char *) question, reqIntLen, timeout);
    if( r < 0 )
    {
          perror("USB control write"); bad("USB write failed"); 
    }

    if(udebug) {
        for (i=0;i<reqIntLen; i++) printf("%02x ",question[i]  & 0xFF);
        printf("\n");
    }
}

void interrupt_transfer(usb_dev_handle *dev) {
 
    int r,i;
    char answer[reqIntLen];
    char question[reqIntLen];
    for (i=0;i<reqIntLen; i++) question[i]=i;
    r = usb_interrupt_write(dev, endpoint_Int_out, question, reqIntLen, timeout);
    if( r < 0 )
    {
          perror("USB interrupt write"); bad("USB write failed"); 
    }
    r = usb_interrupt_read(dev, endpoint_Int_in, answer, reqIntLen, timeout);
    if( r != reqIntLen )
    {
          perror("USB interrupt read"); bad("USB read failed"); 
    }

    if(udebug) {
       for (i=0;i<reqIntLen; i++) printf("%i, %i, \n",question[i],answer[i]);
    }
 
    usb_release_interface(dev, 0);
}

void interrupt_read(usb_dev_handle *dev) {
 
    int r,i;
    char answer[reqIntLen];
    bzero(answer, reqIntLen);
    
    r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
    if( r != reqIntLen )
    {
          perror("USB interrupt read"); bad("USB read failed"); 
    }

    if(udebug) {
       for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
       printf("\n");
    }
}

void interrupt_read_temperatura(usb_dev_handle *dev, float *tempC) {
 
    int r,i, temperature;
    char answer[reqIntLen];
    bzero(answer, reqIntLen);
    
    r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
    if( r != reqIntLen )
    {
          perror("USB interrupt read"); bad("USB read failed"); 
    }


    if(udebug) {
      for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
      printf("\n");
    }
    
    temperature = (answer[3] & 0xFF) + (answer[2] << 8);
    temperature += calibration;
    *tempC = temperature * (125.0 / 32000.0);

}

void bulk_transfer(usb_dev_handle *dev) {
 
    int r,i;
    char answer[reqBulkLen];

    r = usb_bulk_write(dev, endpoint_Bulk_out, NULL, 0, timeout);
    if( r < 0 )
    {
          perror("USB bulk write"); bad("USB write failed"); 
    }
    r = usb_bulk_read(dev, endpoint_Bulk_in, answer, reqBulkLen, timeout);
    if( r != reqBulkLen )
    {
          perror("USB bulk read"); bad("USB read failed"); 
    }


    if(udebug) {
      for (i=0;i<reqBulkLen; i++) printf("%02x ",answer[i]  & 0xFF);
    }
 
    usb_release_interface(dev, 0);
}

float sensor_getTemp() {
  float temperature;
  control_transfer(lvr_winusb, uTemperatura );
  interrupt_read_temperatura(lvr_winusb, &temperature);
  debug("[TEMPer1 sensor plugin] sensor_getTemp %f\n",temperature);
  
  return temperature;
} 

void sensor_initfunc(char *cfgfile) {
  int device = 0;
  
  // read actuator specific configuration options
  device=ini_getl("sensor_plugin_temper1","sensor",0, cfgfile); 
 
  if ((lvr_winusb = setup_libusb_access(device)) == NULL)
    die("[TEMPer1 sensor plugin] OK unable to access sensor %d...\n",device);

  ini_control_transfer(lvr_winusb);
     
  control_transfer(lvr_winusb, uTemperatura );
  interrupt_read(lvr_winusb);
 
  control_transfer(lvr_winusb, uIni1 );
  interrupt_read(lvr_winusb);
 
  control_transfer(lvr_winusb, uIni2 );
  interrupt_read(lvr_winusb);
  interrupt_read(lvr_winusb); 
  debug("[TEMPer1 sensor plugin] OK using sensor %d...\n",device);
}
