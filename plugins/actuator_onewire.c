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

actuator plugin for setting 1-wire actuators via owfs

*/

#include <unistd.h>
#include <stdbool.h>
#include <owcapi.h>
#include <string.h>
#include "myexec.h"
#include "minIni.h"
#include "owactuators.h"
#include "cfgdflt.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

struct s_w1_act_cfg {
  char device[2][16];
  char port[2][6];
  char owparms[100];
};

static struct s_w1_act_cfg w1_act_cfg;

extern bool simulation;
extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
extern void errorlog(char* fmt, ...);
extern bool ow_init_called;

static ssize_t do_OW_init() {
 debug("[onewire actuator plugin] calling OW_init(\"%s\")\n",w1_act_cfg.owparms);
 return OW_init(w1_act_cfg.owparms);
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

static int search4Actuator(char *device,char *port) {
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
        die("[onewire actuator plugin] OW_init failed.\n"); 
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
  
  if (ow_init_called==false) {
    ow_init_called=true;
    ini_gets("control", "owparms", "localhost:4304", w1_act_cfg.owparms,
              sizearray(w1_act_cfg.owparms), cfgfile);
    if(do_OW_init() !=0) {
      errorlog("[onewire actuator plugin] OW_init failed falling back to simulation mode!\n");
      simulation=true;
      return;  
    }
  }

  if (devno==0) {
    // read actuator specific configuration options
    ini_gets("actuator_plugin_onewire", "actuator", "", w1_act_cfg.device[0],
              sizearray(w1_act_cfg.device[0]), cfgfile);
    ini_gets("actuator_plugin_onewire", "actuator_port", "PIO.A", w1_act_cfg.port[0],
              sizearray(w1_act_cfg.port[0]), cfgfile);
  
    // check if actuator is a valid one
    atype=search4Actuator(w1_act_cfg.device[0],w1_act_cfg.port[0]);
    if (-1==atype) {
      errorlog("[onewire actuator plugin] %s/%s is unavailable or not a supported actuator or actuator_port:\n",
        w1_act_cfg.device[0],w1_act_cfg.port[0]);
      errorlog("[onewire actuator plugin] switching to simulation mode!\n");
      simulation=true;
    } else {
      debug("[onewire actuator plugin] OK, found actuator of type %s (id %s, port %s)...\n",
        actuators[atype],w1_act_cfg.device[0],w1_act_cfg.port[0]);
    }
  }
  
  if (devno==1) {
    // read actuator specific configuration options
    ini_gets("actuator_plugin_onewire", "stirring_device", "",
              w1_act_cfg.device[1], sizearray(w1_act_cfg.device[1]), cfgfile);
    ini_gets("actuator_plugin_onewire", "stirring_device_port", "PIO.B",
              w1_act_cfg.port[1], sizearray(w1_act_cfg.port[1]), cfgfile);
              
    // check if stirring_device is a valid one
    if (-1==search4Actuator(w1_act_cfg.device[1],w1_act_cfg.port[1])) {
      errorlog("[onewire actuator plugin] %s/%s is unavailable or not a supported actuator or actuator_port:\n",
      w1_act_cfg.device[1],w1_act_cfg.port[1]);
      errorlog("[onewire actuator plugin] switching to simulation mode!\n");
      simulation=true;
    } else {
      debug("[onewire actuator plugin] OK, found stirring_device actuator of type %s (id %s, port %s)...\n",
        actuators[atype],w1_act_cfg.device[1],w1_act_cfg.port[1]);
    }
  }
}

void actuator_setstate(int devno, int state) {
    debug("[onewire actuator plugin] actuator_setstate\n");
    debug("[onewire actuator plugin] device %s port %s setOWRelay(%d,%d)\n",
    w1_act_cfg.device[devno],w1_act_cfg.port[devno],devno,state);
    setOWRelay(devno, state);
}