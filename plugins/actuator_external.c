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
#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

// defaults
#define EXTACTON "sispmctl -o 1 -d 0"
#define EXTACTOFF "sispmctl -f 1 -d 0"
#define EXTSTIRON "sispmctl -o 2 -d 0"
#define EXTSTIROFF "sispmctl -f 2 -d 0"

struct s_ext_act_cfg {
  char extactuatoron[2][255];
  char extactuatoroff[2][255];
};

static struct s_ext_act_cfg ext_act_cfg;

extern bool simulation;
extern void debug(char* fmt, ...);
extern void errorlog(char* fmt, ...);

void actuator_initfunc(char *cfgfile, int devno) {
  char cmd[255];
  char *cmd_with_path;
  char *cmd_only;
  
  debug("[external actuator plugin] actuator_initfunc device %d\n",devno);

  if (devno==0) {
    ini_gets("actuator_plugin_external", "actuator_on", EXTACTON, ext_act_cfg.extactuatoron[0],
              sizearray(ext_act_cfg.extactuatoron[0]), cfgfile);
    ini_gets("actuator_plugin_external", "actuator_off", EXTACTOFF, ext_act_cfg.extactuatoroff[0],
              sizearray(ext_act_cfg.extactuatoroff[0]), cfgfile);
  }
  if (devno==1) {
    ini_gets("actuator_plugin_external", "stirring_device_on", EXTSTIRON,
              ext_act_cfg.extactuatoron[1], sizearray(ext_act_cfg.extactuatoron[1]), cfgfile);
    ini_gets("actuator_plugin_external", "stirring_device_off", EXTSTIRON,
              ext_act_cfg.extactuatoroff[1], sizearray(ext_act_cfg.extactuatoroff[1]), cfgfile);
  }
  // check if command is available in PATH
  strcpy(cmd,ext_act_cfg.extactuatoron[devno]);
  cmd_only=strtok(cmd," ");
  if (0!=searchXfile(cmd_only,&cmd_with_path)) {
    errorlog("[external actuator plugin] can not find command >%s<:\n",cmd_only);
    errorlog("[external actuator plugin] falling back to simulation mode\n",cmd_only);
    simulation=true;
    free(cmd_with_path);
  }
  free(cmd_with_path);
}

void actuator_setstate(int devno, int state) {
  if (state) {
      debug("[external actuator plugin] device %d running command: %s\n",devno,ext_act_cfg.extactuatoron[devno]);
      myexec(ext_act_cfg.extactuatoron[devno],1);
    } else {
      debug("[external actuator plugin] device %d running command: %s\n",devno,ext_act_cfg.extactuatoroff[devno]);
      myexec(ext_act_cfg.extactuatoroff[devno],1);
    }
}
