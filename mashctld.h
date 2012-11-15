/* 

ctrld

a web-controllable two-level temperature controller for 1-wire
sensor (DS18S20/DS18B20) and various actuators

(c) 2011-2012 Sven Geggus <sven-web20mash@geggus.net>

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
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <arpa/inet.h>
#ifndef NO1W
#include <owcapi.h>
#endif
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <time.h>
#include <limits.h>
#include <magic.h>
#include <signal.h>
#include <syslog.h>
#include <pwd.h>
#include "minIni.h"
#include "cmdline.h"

#define DELAY 4

#define MINTEMP -55
#define MAXTEMP 125
#define MAXTIME 180
#define UNUSED(expr) do { (void)(expr); } while (0)

#define DEFAULTLANG "en"

#define ACT_COOLER 0
#define ACT_HEATER 1

// values for simulation of temperature measurement
// start with this value
#define SIM_INIT_TEMP 42
// increase temperature by this interval on every control interval
#define SIM_INC 0.5

void outSensorActuatorList();
int doControl();
void setRelay(int state);
int loadtemplate(char *filename,char **data);
float getTemp();
int search4Sensor();
int search4Actuator();
void readconfig(char *configfile);
void errorlog(char* fmt, ...);
void die(char* fmt, ...);
void debug(char* fmt, ...);

struct configopts {
  uint16_t port;
  char owparms[100];
  char sensor[16];
  char actuator[16];
  char actuator_port[6];
  bool extactuator;
  char extactuatoron[255];
  char extactuatoroff[255];
  float tempMust;
  float hysteresis;
  int acttype;
  char webroot[255];
  float resttemp[4];
  uint64_t resttime[4];
  char username[255];
  char password[255];
  bool authactive;
  char state_change_cmd[255];
};

struct processstate {
  float tempCurrent;
  float tempMust;
  time_t resttime;
  uint64_t starttime;
  bool relay;
  int mash;
  bool control;
  bool ttrigger;
};
