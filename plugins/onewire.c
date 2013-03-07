/* 

mashctld

a web-controllable two-level temperature and mash process
controler for 1-wire sensor (DS18S20/DS18B20) and various actuators

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

actuator plugin for setting 1-wire actuators via owfs

*/

#include <unistd.h>
#include <stdbool.h>
#include <owcapi.h>
#include <string.h>
#include "myexec.h"
#include "minIni.h"
#include "actuators.h"
#include "cfgdflt.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

struct s_w1_act_cfg {
  char device[2][16];
  char port[2][6];
  char owparms[100];
};

static struct s_w1_act_cfg w1_act_cfg;

extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
extern int search4Device(char *device, const char **devlist);
extern int stringInArray(char * str, const char **arr);
extern void errorlog(char* fmt, ...);
extern size_t do_OW_init();

int search4Actuator(char *device,char *port) {
  int devno;
  devno=search4Device(device,actuators);
  // if a supported actuator has been found check if the given port ID
  // is valid for this type of sensor
  if (devno>-1) {
    if (-1 == stringInArray(port,actuator_ports[devno]))
      return -1;
  }
  return devno;
}

static void setOWRelay(int devno,int state) {
  char cstate[2];
  char buf[255];
  int i;

  if (state) {
    cstate[0]='1';
  } else {
    cstate[0]='0';
  }
  cstate[1]='\0';

  sprintf(buf,"/%s/%s",w1_act_cfg.device[devno],w1_act_cfg.port[devno]);
  for (i=0;i<60;i++) {
    if (-1==OW_put(buf,cstate,strlen(buf))) {
      errorlog("owfs WRITE error, retrying in 1 seconds\n");
      OW_finish();
      sleep(2);
      if(do_OW_init() !=0)
	die("Error connecting owserver on %s\n",w1_act_cfg.owparms);
    } else {
      break;
    }
  }
  if (i==60)
    die("still got a write error after retrying 30 times\n");
  return;
}

void actuator_initfunc(char *cfgfile, int devno) {
  int atype;

  debug("[onewire actuator plugin] actuator_initfunc device %d\n",devno);

  if (devno==0) {
    // read actuator specific configuration options
    ini_gets("actuator_plugin_onewire", "actuator", "", w1_act_cfg.device[0],
              sizearray(w1_act_cfg.device[0]), cfgfile);
    ini_gets("actuator_plugin_onewire", "actuator_port", "PIO.A", w1_act_cfg.port[0],
              sizearray(w1_act_cfg.port[0]), cfgfile);
  
    // check if actuator is a valid one
    atype=search4Actuator(w1_act_cfg.device[0],w1_act_cfg.port[0]);
    if (-1==atype)
      die("[onewire actuator plugin] %s/%s is unavailable or not a supported actuator or actuator_port\n",
        w1_act_cfg.device[0],w1_act_cfg.port[0]);
    else
      debug("[onewire actuator plugin] OK, found actuator of type %s (id %s, port %s)...\n",
        actuators[atype],w1_act_cfg.device[0],w1_act_cfg.port[0]);
  }
  
  if (devno==1) {
    // read actuator specific configuration options
    ini_gets("actuator_plugin_onewire", "stirring_device", "",
              w1_act_cfg.device[1], sizearray(w1_act_cfg.device[1]), cfgfile);
    ini_gets("actuator_plugin_onewire", "stirring_device_port", "PIO.B",
              w1_act_cfg.port[1], sizearray(w1_act_cfg.port[1]), cfgfile);
              
    // check if stirring_device is a valid one
    if (-1==search4Actuator(w1_act_cfg.device[1],w1_act_cfg.port[1]))
      die("[onewire actuator plugin] %s/%s is unavailable or not a supported actuator or actuator_port\n",
      w1_act_cfg.device[1],w1_act_cfg.port[1]);
    else
      debug("[onewire actuator plugin] OK, found stirring_device actuator of type %s (id %s, port %s)...\n",
        actuators[atype],w1_act_cfg.device[1],w1_act_cfg.port[1]);    
  }
}

void actuator_setstate(int devno, int state) {
    debug("[onewire actuator plugin] actuator_setstate\n");
    debug("[onewire actuator plugin] device %s port %s setOWRelay(%d,%d)\n",
    w1_act_cfg.device[devno],w1_act_cfg.port[devno],devno,state);
    setOWRelay(devno, state);
}
