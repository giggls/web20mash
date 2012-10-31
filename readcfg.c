#include "mashctld.h"
#include "cfgdflt.h"

extern struct configopts cfopts;

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

void readconfig(char *cfgfile) {
  int i;
  unsigned defrtime[4] = CTLD_RESTTIMES;
  float defrtemp[4] = CTLD_RESTTEMP;
  char cfresttemp[]="resttempX";
  char cfresttime[]="resttimeX";
  char acttype[7];

  cfopts.port=ini_getl("global","port",CTLD_PORT, cfgfile);  

  ini_gets("auth", "username", CTLD_USERNAME, cfopts.username, sizearray(cfopts.username), cfgfile);
  ini_gets("auth", "password", CTLD_PASSWORD, cfopts.password, sizearray(cfopts.password), cfgfile);
  cfopts.authactive=ini_getbool("auth", "active",CTLD_AUTHACTIVE, cfgfile);
  ini_gets("control", "owparms", CTLD_OWPARMS, cfopts.owparms, sizearray(cfopts.owparms), cfgfile);
  ini_gets("control", "sensor", CTLD_SENSORID, cfopts.sensor, sizearray(cfopts.sensor), cfgfile);
  ini_gets("control", "actuator", CTLD_ACTUATORID, cfopts.actuator, sizearray(cfopts.actuator), cfgfile);

  if (0==strcmp("external",cfopts.actuator)) {
    cfopts.extactuator=true;
    ini_gets("control", "extactuatoron", CTLD_EXTACTON, cfopts.extactuatoron,
	     sizearray(cfopts.extactuatoron), cfgfile);
    ini_gets("control", "extactuatoroff", CTLD_EXTACTOFF, cfopts.extactuatoroff,
	     sizearray(cfopts.extactuatoroff), cfgfile);
  } else {
    cfopts.extactuator=false;
  }
  
  ini_gets("control", "actuatortype", CTLD_ACTTYPE, acttype, sizearray(acttype), cfgfile);
  if (0==strcmp("cooler",acttype)) {
    cfopts.acttype=ACT_COOLER;
  } else {
    cfopts.acttype=ACT_HEATER;
  }
  
  cfopts.tempMust=ini_getf("control", "tempMust", CTLD_TEMPMUST, cfgfile);
  cfopts.hysteresis=ini_getf("control", "hysteresis", CTLD_HYSTERESIS, cfgfile);
  ini_gets("global", "webroot", CTLD_WEBROOT, cfopts.webroot, sizearray(cfopts.webroot), cfgfile);

  // read rest times and temperatures
  for (i=0;i<3;i++) {
    cfresttemp[8]='1'+i;
    cfopts.resttemp[i]=ini_getf("mash-process",cfresttemp,defrtemp[i], cfgfile);
    cfresttime[8]='1'+i;
    cfopts.resttime[i]=ini_getl("mash-process",cfresttime,defrtime[i], cfgfile);
  }
  cfopts.resttemp[i]=ini_getf("mash-process","lauteringtemp",defrtemp[i], cfgfile);

  ini_gets("mash-process", "state_change_cmd", "", cfopts.state_change_cmd,
	     sizearray(cfopts.state_change_cmd), cfgfile);

  if (cfopts.state_change_cmd[0]!='\0')
    // check for %d in state_change_cmd string
    if (NULL==strstr(cfopts.state_change_cmd,"%d"))
      die("invalid value for state_change_cmd (%%d missing)!\n");
}
