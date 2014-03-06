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

actuator plugin for calling external commands

*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "myexec.h"
#include "minIni.h"
#include "searchXfile.h"
#include "errorcodes.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

// defaults
#define EXTACTON "sispmctl -o 1 -d 0"
#define EXTACTOFF "sispmctl -f 1 -d 0"
#define EXTSTIRON "sispmctl -o 2 -d 0"
#define EXTSTIROFF "sispmctl -f 2 -d 0"

#define PREFIX "[external actuator plugin] "

struct s_ext_act_cfg {
  char extactuatoron[2][255];
  char extactuatoroff[2][255];
  char extactuatorcheck[2][255];
};

static struct s_ext_act_cfg ext_act_cfg;

static char plugin_name[]="actuator_external";

// default is no error
static int plugin_error[2]={0,0};

extern bool actuator_simul[2];
extern void debug(char* fmt, ...);
extern void errorlog(char* fmt, ...);
extern void die(char* fmt, ...);

void checkcmd(char *cmdline,int devno,int err) {
  char cmd[255];
  char *cmd_only;
  char *cmd_with_path;
  
  strcpy(cmd,cmdline);
  cmd_only=strtok(cmd," ");
  if (0!=searchXfile(cmd_only,&cmd_with_path)) {
    errorlog(PREFIX"can not find command >%s<:\n",cmd_only);
    errorlog(PREFIX"falling back to simulation mode\n",cmd_only);
    actuator_simul[devno]=true;
    plugin_error[devno]=err;
  } else {
    free(cmd_with_path);
  }
}


void actuator_initfunc(char *cfgfile, int devno) {
  
  debug(PREFIX"actuator_initfunc device %d\n",devno);

  if (devno==0) {
    ini_gets("actuator_plugin_external", "actuator_on", EXTACTON, ext_act_cfg.extactuatoron[0],
              sizearray(ext_act_cfg.extactuatoron[0]), cfgfile);
    ini_gets("actuator_plugin_external", "actuator_off", EXTACTOFF, ext_act_cfg.extactuatoroff[0],
              sizearray(ext_act_cfg.extactuatoroff[0]), cfgfile);
    ini_gets("actuator_plugin_external", "actuator_check", "", ext_act_cfg.extactuatorcheck[0],
              sizearray(ext_act_cfg.extactuatorcheck[0]), cfgfile);
            
  }
  if (devno==1) {
    ini_gets("actuator_plugin_external", "stirring_device_on", EXTSTIRON,
              ext_act_cfg.extactuatoron[1], sizearray(ext_act_cfg.extactuatoron[1]), cfgfile);
    ini_gets("actuator_plugin_external", "stirring_device_off", EXTSTIRON,
              ext_act_cfg.extactuatoroff[1], sizearray(ext_act_cfg.extactuatoroff[1]), cfgfile);
    ini_gets("actuator_plugin_external", "stirring_device_check", "",
              ext_act_cfg.extactuatorcheck[1], sizearray(ext_act_cfg.extactuatorcheck[1]), cfgfile);
  }

  // check if on and off commands are available in PATH
  if (!actuator_simul[devno]) {
    checkcmd(ext_act_cfg.extactuatoron[devno],devno,ERROR_ON_CMDNOTFOUND);
    if (!actuator_simul[devno]) {
      checkcmd(ext_act_cfg.extactuatoroff[devno],devno,ERROR_OFF_CMDNOTFOUND);
    }
  }
  
  if (!actuator_simul[devno]) {
    if (ext_act_cfg.extactuatorcheck[devno][0]!='\0') {
       checkcmd(ext_act_cfg.extactuatorcheck[devno],devno,ERROR_CHECK_CMDNOTFOUND);
       // try to exec check command if a valid one was given in configfile
       if (!actuator_simul[devno]) {
         debug(PREFIX"device %d calling check command: %s\n",devno,ext_act_cfg.extactuatorcheck[devno]);
         if (0!=myexec(ext_act_cfg.extactuatorcheck[devno],true,true,1)) {
           errorlog(PREFIX"device %d check command \"%s\" failed, falling back to simulation mode\n",devno,ext_act_cfg.extactuatorcheck[devno]);
           plugin_error[devno]=ERROR_CHECK_CMDFAILED;
           actuator_simul[devno]=true;
         }
       }
    }
  }
}

void actuator_setstate(int devno, int state) {
  if (state) {
      debug(PREFIX"device %d running command: %s\n",devno,ext_act_cfg.extactuatoron[devno]);
      myexec(ext_act_cfg.extactuatoron[devno],false,false,1);
    } else {
      debug(PREFIX"device %d running command: %s\n",devno,ext_act_cfg.extactuatoroff[devno]);
      myexec(ext_act_cfg.extactuatoroff[devno],false,false,1);
    }
}

size_t actuator_getInfo(int devno, size_t max, char *buf) {
  size_t pos, rest;
  
  debug(PREFIX"actuator_getInfo called\n");
  
  pos=snprintf(buf,max,
  "  {\n"
  "    \"type\": \"actuator\",\n"
  "    \"name\": \"%s\",\n"
  "    \"device\": \"%d\",\n"
  "    \"error\": \"%d\",\n",
  plugin_name, devno,plugin_error[devno]);
  if (pos >=max) {
    buf[0]='\0';
    return(0);
  }
  rest=max-pos;
  if (devno==0) {
    pos+=snprintf(buf+pos,rest,
                  "    \"devlist\": [],\n"
                  "    \"options\": [\"actuator_on = %s\",\"actuator_off = %s\",\"actuator_check = %s\"]\n",
                  ext_act_cfg.extactuatoron[devno],ext_act_cfg.extactuatoroff[devno],
                  ext_act_cfg.extactuatorcheck[devno]);
  } else {
    pos+=snprintf(buf+pos,rest,
                  "    \"devlist\": [],\n"
                  "    \"options\": [\"stirring_device_on = %s\",\"stirring_device_off = %s\",\"stirring_device_check = %s\"]\n",
                  ext_act_cfg.extactuatoron[devno],ext_act_cfg.extactuatoroff[devno],
                  ext_act_cfg.extactuatorcheck[devno]);
  }       
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
