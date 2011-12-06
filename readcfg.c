#include "mashctld.h"
#include "cfgdflt.h"

extern struct configopts cfopts;

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

void readconfig(char *cfgfile) {
  int i;
  unsigned defrtime[3] = CTLD_RESTTIMES;
  float defrtemp[3] = CTLD_RESTTEMP;
  char cfresttemp[]="resttempX";
  char cfresttime[]="resttimeX";

  cfopts.port=ini_getl("global","port",CTLD_PORT, cfgfile);  
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
  cfopts.tempMust=ini_getf("control", "tempMust", CTLD_TEMPMUST, cfgfile);
  cfopts.hysteresis=ini_getf("control", "hysteresis", CTLD_HYSTERESIS, cfgfile);
  cfopts.heater=ini_getbool("control", "heater",CTLD_HEATER, cfgfile);
  ini_gets("global", "webroot", CTLD_WEBROOT, cfopts.webroot, sizearray(cfopts.webroot), cfgfile);

  // read rest times and temperatures
  for (i=0;i<3;i++) {
    cfresttemp[8]='1'+i;
    cfopts.resttemp[i]=ini_getf("mash-process",cfresttemp,defrtemp[i], cfgfile);
    cfresttime[8]='1'+i;
    cfopts.resttime[i]=ini_getl("mash-process",cfresttime,defrtime[i], cfgfile);
  }

}
