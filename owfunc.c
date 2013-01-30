/* 

mashctld

a web-controllable two-level temperature and mash process
controler for 1-wire sensor (DS18S20/DS18B20) and various actuators

(c) 2011-2012 Sven Geggus <sven-web20mash@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

1-wire & control functions

*/
#include "mashctld.h"
#include "sensact.h"

extern struct configopts cfopts;
extern struct processstate pstate;
extern char cfgfp[PATH_MAX + 1];
extern int gpiofd;

/* clig command line Parameters*/  
extern Cmdline *cmd;

int stringInArray(char * str, const char **arr) {
  const char **i;
  int c=0;
  for (i=arr; *i; i++) {
    if (0==strcmp(*i,str))
      return c;
    c++;
  }
  return -1;
}

#ifndef NO1W
void outSensorActuatorList() {
  char buf1[255],buf2[255];
  char *s1,*s2,*tok;
  const char **i;
  bool first=1;
  size_t slen1,slen2;
  int pos;

  OW_get("/",&s1,&slen1);
    
  tok=strtok(s1,",");
  printf("sensors:\n");
  while (tok != NULL) {
    /* if a sensor id is found scan for a supported sensor */
    if (tok[2] == '.') {
      sprintf(buf1,"/%stype",tok);
      OW_get(buf1,&s2,&slen2);
      pos=stringInArray(s2,sensors);
      if (-1 != pos) {
	tok[strlen(tok)-1]=0;
	printf("%s: %s\n",sensors[pos],tok);
	if (cmd->writeP && first) {
	  ini_puts("control", "sensor", tok, cfgfp);
	  first=0;
        }
      }
    }
    tok=strtok(NULL,",");
  }
  free(s1);
  free(s2);
  
  first=1;
  OW_get("/",&s1,&slen1);
  
  tok=strtok(s1,",");
  printf("\nactuators:\n");
  while (tok != NULL) {
    /* if an actuators id is found scan for type */
    if (tok[2] == '.') {
      sprintf(buf1,"/%stype",tok);
      OW_get(buf1,&s2,&slen2);
      pos = stringInArray(s2,actuators);
      if (-1 != pos) {
	tok[strlen(tok)-1]=0;
	printf("%s:",actuators[pos]);
	for (i=actuator_ports[pos]; *i; i++)
	  printf(" %s/%s",tok,*i);
        printf("\n");
	if (cmd->writeP && first) { 
	  strcpy(buf2,tok);
	  strcpy(buf2+strlen(tok),"/");
	  strcpy(buf2+strlen(tok)+1,actuator_ports[pos][0]);
	  ini_puts("control", "actuator", buf2, cfgfp);
	  first=0;
	}
      }
    }
    tok=strtok(NULL,",");
  }
  free(s1);
  free(s2);
}

int search4Device(char *device, const char **devlist) {
  
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


/* check if sensor device with given ID and known type is available on the bus */
int search4Sensor() {
  return search4Device(cfopts.sensor,sensors);
}

int search4Actuator() {
  int devno;
  devno=search4Device(cfopts.actuator[0],actuators);
  // if a supported actuator has been found check if the given port ID
  // is valid for this type of sensor
  if (devno>-1) {
    if (-1 == stringInArray(cfopts.actuator_port[0],actuator_ports[devno]))
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

  sprintf(buf,"/%s/%s",cfopts.actuator[devno],cfopts.actuator_port[devno]);
  for (i=0;i<60;i++) {
    if (-1==OW_put(buf,cstate,strlen(buf))) {
      errorlog("owfs WRITE error, retrying in 1 seconds\n");
      OW_finish();
      sleep(2);
      if(OW_init(cfopts.owparms) !=0)
	die("Error connecting owserver on %s\n",cfopts.owparms);
    } else {
      break;
    }
  }
  if (i==60)
    die("still got a write error after retrying 30 times\n");
  return;
}
#endif

/* two level control function, assume control actuator to be always device number 0 */
int doTempControl() {
  float ubarrier;
  float lbarrier;

  if (ACT_HEATER==cfopts.acttype) {
    ubarrier=pstate.tempMust;
    lbarrier=pstate.tempMust-(cfopts.hysteresis);
    if (pstate.tempCurrent < lbarrier) {
      if (pstate.relay[0]==0) {
	setRelay(0,1);
      }
    } 
    if (pstate.tempCurrent >= ubarrier) {
      if (pstate.relay[0]==1) {
	setRelay(0,0);
      }
    }
  } else {
    ubarrier=pstate.tempMust+(cfopts.hysteresis);
    lbarrier=pstate.tempMust;
    if (pstate.tempCurrent > ubarrier) {
      if (pstate.relay[0]==0) {
	setRelay(0,1);
      }
    }
    if (pstate.tempCurrent <=  lbarrier) {
      if (pstate.relay[0]==1) {
	setRelay(0,0);
      }
    }
  }

  return(0);
}

#ifndef NO1W
float getTemp() {
  float temperature;
  char *s;
  size_t slen;
  char buf[255];
  unsigned i;

  /* acquire current temperature value 
       we try again up to 30 times if we get a read-error
  */
  sprintf(buf,"/uncached/%s/temperature",cfopts.sensor);
  for (i=0;i<60;i++) {
    if (-1==OW_get(buf,&s,&slen)) {
      errorlog("owfs READ error, retrying in 1 second\n");
      free(s);
      OW_finish();
      sleep(3);
      if(OW_init(cfopts.owparms) !=0)
	die("Error connecting owserver on %s\n",cfopts.owparms);
      sleep(1);
    } else {
      break;
    }
  }
  if (i==60)
    die("still got a read error after retrying 30 times\n");

  sscanf(s,"%f",&temperature);
  free(s);
  return temperature;
}
#endif

void setRelay(int devno, int state) {
  if (!cmd->simulationP) {
    if (cfopts.extactuator[devno]) {
      if (state) {
        if (cmd->debugP)
	  debug("running external actuator command: %s\n",cfopts.extactuatoron[devno]);
	  system(cfopts.extactuatoron[devno]);
      } else {
        if (cmd->debugP)
	  debug("running external actuator command: %s\n",cfopts.extactuatoroff[devno]);
	  system(cfopts.extactuatoroff[devno]);
      }
    } else {
      if (cfopts.gpioactuator[devno]) {
        if (state)
         write(gpiofd,"1",1);
        else
         write(gpiofd,"0",1);
#ifndef NO1W
      } else {   
        setOWRelay(devno,state);
#endif
      }
    }
  }
  pstate.relay[devno]=state;
}
