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

gpio plugin for actuator control using linux gpio ans sysfs

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "myexec.h"
#include "minIni.h"
#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

// defaults
#define ACTUATOR "/sys/class/gpio/gpio25/value"
#define STIRDEV "/sys/class/gpio/gpio26/value"

struct s_gpio_act_cfg {
  char name[2][1024];
  int fd[2];
};

static struct s_gpio_act_cfg gpio_act_cfg;

extern void debug(char* fmt, ...);
extern void die(char* fmt, ...);

void actuator_initfunc(char *cfgfile, int devno) {

  debug("[gpio actuator plugin] actuator_initfunc\n");

  if (devno==0) {
    ini_gets("actuator_plugin_gpio", "actuator", ACTUATOR, gpio_act_cfg.name[0],
              sizearray(gpio_act_cfg.name[0]), cfgfile);
  
    gpio_act_cfg.fd[0] = open(gpio_act_cfg.name[0], O_RDWR);
    if (gpio_act_cfg.fd[0] < 0)
      die("unable to open GPIO device >%s<\n",gpio_act_cfg.name[0]);
  }
  
  if (devno==1) {
    ini_gets("actuator_plugin_gpio", "stirring_device", STIRDEV, gpio_act_cfg.name[1],
              sizearray(gpio_act_cfg.name[1]), cfgfile);
  
    gpio_act_cfg.fd[1] = open(gpio_act_cfg.name[1], O_RDWR);
    if (gpio_act_cfg.fd[1] < 0)
      die("unable to open GPIO device >%s<\n",gpio_act_cfg.name[1]);
  }
    
}

void actuator_setstate(int devno, int state) {
      debug("[gpio actuator plugin] setting device %d to %d\n",devno,state);
      if (state)
        write(gpio_act_cfg.fd[devno],"1",1);
      else
        write(gpio_act_cfg.fd[devno],"0",1);
}
