/* 

ctrld

a web-controllable two-level temperature controller for 1-wire
sensor (DS18S20) and various actuators

(c) 2011 Sven Geggus <sven@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

main header file

*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <owcapi.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <time.h>
#include <limits.h>
#include <magic.h>
#include "minIni.h"
#include "cmdline.h"

#define DELAY 4

#define MINTEMP -55
#define MAXTEMP 125
#define UNUSED(expr) do { (void)(expr); } while (0)

void printSensorActuatorList();
int doControl();
void setRelais(int state);
int loadtemplate(char *filename,char **data);
float getTemp();
bool search4Device(char *id, char *type);
void readconfig(char *configfile);

struct configopts {
  uint16_t port;
  char owparms[100];
  char sensor[20];
  char actuator[20];
  bool extactuator;
  char extactuatoron[255];
  char extactuatoroff[255];
  float tempMust;
  float hysteresis;
  bool heater;
  char rtcdev[20];
  char webroot[255];
  float resttemp[3];
  uint64_t resttime[3];
};

struct processstate {
  float tempCurrent;
  float tempMust;
  time_t resttime;
  uint64_t starttime;
  bool relais;
  int mash;
  bool control;
  bool ttrigger;
};
