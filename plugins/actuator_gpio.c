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

gpio plugin for actuator control using linux gpio ans sysfs

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include "myexec.h"
#include "minIni.h"
#include "errorcodes.h"

#define sizearray(a)    (sizeof(a) / sizeof((a)[0]))

static char plugin_name[]="actuator_gpio";

// default is no error  
static int plugin_error[2]={0,0};

// defaults
#define ACTUATOR "/sys/class/gpio/gpio25/value"
#define STIRDEV "/sys/class/gpio/gpio26/value"
#define BASEDIR "/sys/class/gpio"

#define PREFIX "[gpio actuator plugin] "

struct s_gpio_act_cfg {
  char name[2][1024];
  int fd[2];
};

static struct s_gpio_act_cfg gpio_act_cfg;

extern bool actuator_simul[2];
extern void debug(char* fmt, ...);
extern void errorlog(char* fmt, ...);

void actuator_initfunc(char *cfgfile, int devno) {

  debug(PREFIX"actuator_initfunc device %d\n",devno);

  if (devno==0) {
    ini_gets("actuator_plugin_gpio", "actuator", ACTUATOR, gpio_act_cfg.name[0],
              sizearray(gpio_act_cfg.name[0]), cfgfile);
  
    gpio_act_cfg.fd[0] = open(gpio_act_cfg.name[0], O_RDWR);
    if (gpio_act_cfg.fd[0] < 0) {
      errorlog(PREFIX"unable to open GPIO device >%s<:\n",gpio_act_cfg.name[0]);
      errorlog(PREFIX"falling back to simluation mode\n");
      actuator_simul[0]=true;
      plugin_error[0]=IOERROR;
      return;
    }
  }
  
  if (devno==1) {
    ini_gets("actuator_plugin_gpio", "stirring_device", STIRDEV, gpio_act_cfg.name[1],
              sizearray(gpio_act_cfg.name[1]), cfgfile);
  
    gpio_act_cfg.fd[1] = open(gpio_act_cfg.name[1], O_RDWR);
    if (gpio_act_cfg.fd[1] < 0) {
      errorlog(PREFIX"unable to open GPIO device >%s<\n",gpio_act_cfg.name[1]);
      errorlog(PREFIX"falling back to simluation mode\n");
      actuator_simul[1]=true;
      plugin_error[1]=IOERROR;
      return;
    }
  }
    
}

void actuator_setstate(int devno, int state) {
      debug(PREFIX"setting device %d to %d\n",devno,state);
      if (state)
        write(gpio_act_cfg.fd[devno],"1",1);
      else
        write(gpio_act_cfg.fd[devno],"0",1);
}

void fill_dirlist(size_t max, char *dirlist) {
  DIR *dp;
  size_t pos,num,space;
  bool overflow;
  char filename[100];
  char direction[4];
  int fd;
  
  struct dirent *ep;
  dp = opendir(BASEDIR);
  if (dp != NULL) {
    space=max;
    pos=0;
    overflow=false;
    while ((ep = readdir(dp))) {
      if (0!=strncmp("gpio",ep->d_name,4)) continue; 
      if (0==strncmp("gpiochip",ep->d_name,8)) continue;
      // only outputs are interesting
      snprintf(filename,100,"%s/%s/direction",BASEDIR,ep->d_name);
      fd=open(filename,O_RDONLY);
      if (fd<0) continue;
      read(fd,direction,4);
      close(fd);
      if (0!=strncmp("out",direction,3)) continue;
      if (!overflow) {
        num=snprintf(dirlist+pos,space,"\"%s/%s/value\",",BASEDIR,ep->d_name);
        if (num>=space) {
          overflow=true;
        }
        pos+=num;  
        space-=num;
      }
    }
    if (!overflow) {
      dirlist[pos-1]='\0';
    } else {
      dirlist[0]='\0';
    }
    closedir(dp);
  } else {
    dirlist[0]='\0';
  }
}

size_t actuator_getInfo(int devno, size_t max, char *buf) {
  size_t pos, rest;
  char dirlist[1024];
  
  debug(PREFIX"actuator_getInfo called\n");
  
  fill_dirlist(1024,dirlist);
  
  pos=snprintf(buf,max,
  "  {\n"\
  "    \"type\": \"actuator\",\n"\
  "    \"name\": \"%s\",\n"\
  "    \"device\": \"%s\",\n"
  "    \"error\": \"%d\",\n",
  plugin_name, gpio_act_cfg.name[devno],plugin_error[devno]);
  if (pos >=max) {
    buf[0]='\0';
    return(0);
  }
  rest=max-pos;
  pos+=snprintf(buf+pos,rest,
                "    \"devlist\": [%s],\n"
                "    \"options\": []\n"
                ,dirlist);
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
