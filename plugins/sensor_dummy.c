/* 

mashctld

a web-controllable two-level temperature and mash process
controler for various sensors and actuators

(c) 2011-2014 Sven Geggus <sven-web20mash@geggus.net>
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

plugin for getting a constant dummy temperature value of 42

*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "errorcodes.h"

#define PREFIX "[dummy sensor plugin] "

#define TEMPERATURE 42.0

static char plugin_name[]="sensor_dummy";

extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);
extern void errorlog(char* fmt, ...);

void sensor_initfunc(char *cfgfile) {
  debug(PREFIX"sensor_initfunc (cfgfile: %s)\n",cfgfile);
  return;
}

float sensor_getTemp() {
  debug(PREFIX"sensor_getTemp %f\n",TEMPERATURE);
  return TEMPERATURE;
}

size_t sensor_getInfo(size_t max, char *buf) {
  
  debug(PREFIX"sensor_getInfo called\n");

  snprintf(buf,max,
    "  {\n"\
    "    \"type\": \"sensor\",\n"\
    "    \"name\": \"%s\",\n"\
    "    \"device\": \"dummy_sensor\",\n"
    "    \"error\": \"0\",\n"
    "    \"devlist\": [\"dummy\"]\n"
    "  }\n",
  plugin_name);
  return(strlen(buf));
}
