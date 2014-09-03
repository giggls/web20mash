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

dummy plugin for actuator control

*/
#include <stdio.h>
#include <string.h>
#include "errorcodes.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

static char plugin_name[]="actuator_dummy";

#define PREFIX "[dummy actuator plugin] "

extern void debug(char* fmt, ...);
extern void errorlog(char* fmt, ...);

void actuator_initfunc(char *cfgfile, int devno) {

  debug(PREFIX"actuator_initfunc device %d (cfgfile: %s)\n",devno,cfgfile);
     
}

void actuator_setstate(int devno, int state) {
      debug(PREFIX"setting device %d to %d\n",devno,state);
}

size_t actuator_getInfo(int devno, size_t max, char *buf) {
  
  debug(PREFIX"actuator_getInfo called (devno: %d)\n",devno);
  
  snprintf(buf,max,
  "  {\n"\
  "    \"type\": \"actuator\",\n"\
  "    \"name\": \"%s\",\n"\
  "    \"device\": \"dummy%d\",\n"
  "    \"error\": \"0\",\n"
  "    \"devlist\": [\"dummy0\",\"dummy1\"],\n"
  "    \"options\": []\n"
  "  }\n",
  plugin_name,devno);
  return(strlen(buf));
}
