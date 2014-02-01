/* 

mashctld

a web-controllable two-level temperature and mash process
controler for various sensors and actuators

(c) 2011-2013 Sven Geggus <sven-web20mash@geggus.net>
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

plugin for getting temperature values from 1-wire sensor
(DS18S20/DS18B20)via owfs

*/

#include <unistd.h>
#include <stdbool.h>
#include <owcapi.h>
#include <string.h>
#include "myexec.h"
#include "minIni.h"
#include "owsensors.h"
#include "cfgdflt.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

static char device[16];
static char owparms[100];

extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
extern void errorlog(char* fmt, ...);
extern bool ow_init_called;

static ssize_t do_OW_init() {
 debug("[onewire sensor plugin] calling OW_init(\"%s\")\n",owparms);
 return OW_init(owparms);
}

static int stringInArray(char * str, const char **arr) {
  const char **i;
  int c=0;
  for (i=arr; *i; i++) {
    if (0==strcmp(*i,str))
      return c;
    c++;
  }
  return -1;
}

static int search4Device(char *device, const char **devlist) {
  
  char curdev[22];
  char *type_found_on_bus;
  size_t slen;
  
  curdev[0]='/';
  strncpy(curdev+1,device,15);
  strcpy(curdev+16,"/type");
  type_found_on_bus=NULL;
  OW_get(curdev,&type_found_on_bus,&slen);
  // return false if requested sensor is not available at all
  if (NULL==type_found_on_bus)  return -1;
  // return false if requested has wrong type
  return stringInArray(type_found_on_bus,devlist);
}

void sensor_initfunc(char *cfgfile) {
  int stype;

  debug("[onewire sensor plugin] actuator_initfunc\n");
  
  if (ow_init_called==false) {
    ow_init_called=true;
    ini_gets("control", "owparms", "localhost:4304", owparms,
              sizearray(owparms), cfgfile);
    do_OW_init();
  }

  // read actuator specific configuration options
  ini_gets("sensor_plugin_onewire", "sensor", "", device,
            sizearray(device), cfgfile);
  
  // check if sensor is a valid one
  stype=search4Device(device,sensors);
  if (-1==stype)
    die("[onewire sensor plugin] %s is unavailable or not a supported sensor\n", device);
  else
    debug("[onewire sensor plugin] OK, found sensor of type %s (id %s)...\n", sensors[stype],device);  
}

float sensor_getTemp() {
  float temperature;
  char *s;
  size_t slen;
  char buf[255];
  unsigned i;

  /* acquire current temperature value 
       we try again up to 30 times if we get a read-error
  */
  sprintf(buf,"/uncached/%s/temperature",device);
  for (i=0;i<60;i++) {
    if (-1==OW_get(buf,&s,&slen)) {
      errorlog("owfs READ error, retrying in 1 second\n");
      free(s);
      OW_finish();
      sleep(3);
      if(do_OW_init() !=0)
	die("Error connecting owserver on %s\n",owparms);
      sleep(1);
    } else {
      break;
    }
  }
  if (i==60)
    die("still got a read error after retrying 30 times\n");

  sscanf(s,"%f",&temperature);
  free(s);
  debug("[onewire sensor plugin] sensor_gettemp %f\n",temperature);
  return temperature;
}
