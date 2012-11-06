/* 

mashctld

a web-controllable two-level temperature and mash process
controler for 1-wire sensor (DS18S20/DS18B20) and various actuators

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

1-wire & control functions

*/
#include "mashctld.h"

extern struct configopts cfopts;
extern struct processstate pstate;
extern char cfgfp[];

/* clig command line Parameters*/  
extern Cmdline *cmd;

void outSensorActuatorList() {
  char buf[255];
  char *s1,*s2,*tok;
  bool first=1;
  size_t slen1,slen2;

  OW_get("/",&s1,&slen1);
    
  tok=strtok(s1,",");
  printf("sensors:\n");
  while (tok != NULL) {
    /* wenn a sensor id is found scan for DS18S20/DS18B20 */
    if (tok[2] == '.') {
      sprintf(buf,"/%stype",tok);
      OW_get(buf,&s2,&slen2);
      if (strcmp(s2,"DS18S20")==0) {
	tok[strlen(tok)-1]=0;
	printf("%s (DS18S20)\n",tok);
	if (cmd->writeP && first) {
          ini_puts("control", "sensor", tok, cfgfp);
          first=0;
	}
      }
      if (strcmp(s2,"DS18B20")==0) {
	tok[strlen(tok)-1]=0;
	printf("%s (DS18B20)\n",tok);
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
  printf("\naktors:\n");
  while (tok != NULL) {
    /* wenn a sensor id is found scan for type */
    if (tok[2] == '.') {
      sprintf(buf,"/%stype",tok);
      OW_get(buf,&s2,&slen2);
      if (strcmp(s2,"DS2406")==0) {
	tok[strlen(tok)-1]=0;
	printf("%s (DS2406)\n",tok);
	if (cmd->writeP && first) { 
	  ini_puts("control", "actuator", tok, cfgfp);
	  first=0;
	}
      }
    }
    tok=strtok(NULL,",");
  }
  free(s1);
  free(s2);
}

/* check if device with given ID and type is available on the bus */
bool search4Device(char *id, char *type) {
  char curdev[22];
  char *type_found_on_bus;
  size_t slen;
  
  curdev[0]='/';
  strncpy(curdev+1,id,15);
  strcpy(curdev+16,"/type");
  type_found_on_bus=NULL;
  OW_get(curdev,&type_found_on_bus,&slen);
  // return false if requested sensor is not available at all
  if (NULL==type_found_on_bus)  return false;
  // return false if requested has wrong type
  if (0==strcmp(type,type_found_on_bus)) {
    return true;
  } else {
    return false;
  }
}

static void setOWRelay(int state) {
  char cstate[2];
  char buf[255];
  int i;

  if (state) {
    cstate[0]='1';
  } else {
    cstate[0]='0';
  }
  cstate[1]='\0';

  sprintf(buf,"/%s/PIO.A",cfopts.actuator);
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
  pstate.relay=state;
  return;
}

int doControl() {
  float ubarrier;
  float lbarrier;

  if (ACT_HEATER==cfopts.acttype) {
    ubarrier=pstate.tempMust;
    lbarrier=pstate.tempMust-(cfopts.hysteresis);
    if (pstate.tempCurrent < lbarrier) {
      if (pstate.relay==0) {
	setRelay(1);
      }
    } 
    if (pstate.tempCurrent >= ubarrier) {
      if (pstate.relay==1) {
	setRelay(0);
      }
    }
  } else {
    ubarrier=pstate.tempMust+(cfopts.hysteresis);
    lbarrier=pstate.tempMust;
    if (pstate.tempCurrent > ubarrier) {
      if (pstate.relay==0) {
	setRelay(1);
      }
    }
    if (pstate.tempCurrent <=  lbarrier) {
      if (pstate.relay==1) {
	setRelay(0);
      }
    }
  }

  return(0);
}

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

void setRelay(int state) {
  if (cfopts.extactuator) {
    if (state) {
      if (cmd->debugP)
	debug("running external actuator command: %s\n",cfopts.extactuatoron);
      system(cfopts.extactuatoron);
    } else {
      if (cmd->debugP)
	debug("running external actuator command: %s\n",cfopts.extactuatoroff);
      system(cfopts.extactuatoroff);
    }
    pstate.relay=state;
  } else {
    setOWRelay(state);
  }
}
