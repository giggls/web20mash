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
#include "errorcodes.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))
#define PREFIX "[onewire actuator plugin] "

static char plugin_name[]="actuator_onewire";

// default is no error  
static int plugin_error[2]={0,0};

struct s_w1_act_cfg {
  char device[2][16];
  char port[2][6];
  char devicetype[2][20];
  char owparms[100];
};

static struct s_w1_act_cfg w1_act_cfg;

extern bool actuator_simul[2];
extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
extern void errorlog(char* fmt, ...);
extern bool ow_init_called;

static ssize_t do_OW_init() {
 debug(PREFIX"calling OW_init(\"%s\")\n",w1_act_cfg.owparms);
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

static int search4Device(int devno, char *device, const char **devlist) {
  
  char curdev[22];
  char *type_found_on_bus;
  size_t slen;
  int res;
  
  curdev[0]='/';
  strncpy(curdev+1,device,15);
  strcpy(curdev+16,"/type");
  type_found_on_bus=NULL;
  OW_get(curdev,&type_found_on_bus,&slen);
  // return false if requested sensor is not available at all
  if (NULL==type_found_on_bus)  return -1;
  // return false if requested has wrong type
  res=stringInArray(type_found_on_bus,devlist);
  if (-1!=res) {
    strcpy(w1_act_cfg.devicetype[devno],type_found_on_bus);
  }
  return res;
}

static int search4Actuator(int devno, char *device,char *port) {
  int lpos;
  lpos=search4Device(devno, device, actuators);
  // if a supported actuator has been found check if the given port ID
  // is valid for this type of sensor
  if (lpos>-1) {
    if (-1 == stringInArray(port,actuator_ports[lpos]))
      return -1;
  }
  return lpos;
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
        die(PREFIX"OW_init failed.\n"); 
    } else {
      break;
    }
  }
  if (i==60)
    die("still got a write error after retrying 30 times\n");
  return;
}

static void queryActuatorList(int max, char *list) {
  char buf[255];
  char *s1,*s2,*tok;
  size_t slen1,slen2;
  int pos,opos,olen1,olen2;
  const char **i;
  bool overflow;
  
  OW_get("/",&s1,&slen1);
  
  opos=0;
  olen2=max;
  tok=strtok(s1,",");
  overflow=false;
  list[0]='\0';
  while (tok != NULL) {
    /* if an actuators id is found scan for type */
    if (tok[2] == '.') {
      sprintf(buf,"/%stype",tok);
      OW_get(buf,&s2,&slen2);
      pos = stringInArray(s2,actuators);
      if (-1 != pos) {
	tok[strlen(tok)-1]=0;
	for (i=actuator_ports[pos]; *i; i++)
	  if (!overflow) {
	    olen1=snprintf(list+opos,olen2,"\"%s: %s/%s\",",actuators[pos],tok,*i);
	    if (olen1>=olen2) {
	      overflow=true;
	    }
	    opos+=olen1;
	    olen2-=olen1;
	  }
      }
    }
    tok=strtok(NULL,",");
  }

  if (!overflow) {
    list[opos-1]='\0';
  }
  free(s1);
  free(s2);                                                                      
  
  list[max-1]='\0';
}

void actuator_initfunc(char *cfgfile, int devno) {
  int atype;

  debug(PREFIX"actuator_initfunc device %d\n",devno);
  
  ini_gets("control", "owparms", "localhost:4304", w1_act_cfg.owparms,
              sizearray(w1_act_cfg.owparms), cfgfile);
  if (ow_init_called==false) {
    if(do_OW_init() !=0) {
      errorlog(PREFIX"OW_init failed falling back to simulation mode!\n");
      actuator_simul[devno]=true;
      plugin_error[devno]=OWINIT_ERROR;
      return;  
    }
    ow_init_called=true;
  }

  if (devno==0) {
    // read actuator specific configuration options
    ini_gets("actuator_plugin_onewire", "actuator", "", w1_act_cfg.device[0],
              sizearray(w1_act_cfg.device[0]), cfgfile);
    ini_gets("actuator_plugin_onewire", "actuator_port", "PIO.A", w1_act_cfg.port[0],
              sizearray(w1_act_cfg.port[0]), cfgfile);
  
    // check if actuator is a valid one
    atype=search4Actuator(devno,w1_act_cfg.device[0],w1_act_cfg.port[0]);
    if (-1==atype) {
      errorlog(PREFIX"%s/%s is unavailable or not a supported actuator or actuator_port:\n",
        w1_act_cfg.device[0],w1_act_cfg.port[0]);
      errorlog(PREFIX"switching to simulation mode!\n");
      actuator_simul[0]=true;
      plugin_error[devno]=ERROR_DEV_NOTFOUND;
    } else {
      debug(PREFIX"OK, found actuator of type %s (id %s, port %s)...\n",
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
    if (-1==search4Actuator(devno,w1_act_cfg.device[1],w1_act_cfg.port[1])) {
      errorlog(PREFIX"%s/%s is unavailable or not a supported actuator or actuator_port:\n",
      w1_act_cfg.device[1],w1_act_cfg.port[1]);
      errorlog(PREFIX"switching to simulation mode!\n");
      actuator_simul[1]=true;
      plugin_error[devno]=ERROR_DEV_NOTFOUND;
    } else {
      debug(PREFIX"OK, found stirring_device actuator of type %s (id %s, port %s)...\n",
        actuators[atype],w1_act_cfg.device[1],w1_act_cfg.port[1]);
    }
  }
}

void actuator_setstate(int devno, int state) {
    debug(PREFIX"actuator_setstate\n");
    debug(PREFIX"device %s port %s setOWRelay(%d,%d)\n",
    w1_act_cfg.device[devno],w1_act_cfg.port[devno],devno,state);
    setOWRelay(devno, state);
}

size_t actuator_getInfo(int devno, size_t max, char *buf) {
  char devlist[1024];
  size_t pos, rest;

  debug(PREFIX"actuator_getInfo called\n");

  if (ow_init_called) {
    queryActuatorList(1024,devlist);
  } else {
    devlist[0]='\0';
  }

  if (plugin_error[devno]==0) {
    pos=snprintf(buf,max,
    "  {\n"
    "    \"type\": \"actuator\",\n"
    "    \"name\": \"%s\",\n"
    "    \"device\": \"%s: %s/%s\",\n"
    "    \"error\": \"%d\",\n",
    plugin_name, w1_act_cfg.devicetype[devno],
    w1_act_cfg.device[devno], w1_act_cfg.port[devno],plugin_error[devno]);
  } else {
    pos=snprintf(buf,max,
    "  {\n"
    "    \"type\": \"actuator\",\n"
    "    \"name\": \"%s\",\n"
    "    \"device\": \"\",\n"
    "    \"error\": \"%d\",\n",
    plugin_name,plugin_error[devno]);
  }
  if (pos >=max) {
    buf[0]='\0';
    return(0);
  }
  rest=max-pos;
  pos+=snprintf(buf+pos,rest,
                "    \"devlist\": [%s],\n"
                "    \"options\": [\"owparms = %s\"]\n",
                devlist,w1_act_cfg.owparms);
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
