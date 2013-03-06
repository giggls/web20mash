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
  char buf[100];

  cfopts.port=ini_getl("global","port",CTLD_PORT, cfgfile);  

  ini_gets("auth", "username", CTLD_USERNAME, cfopts.username, sizearray(cfopts.username), cfgfile);
  ini_gets("auth", "password", CTLD_PASSWORD, cfopts.password, sizearray(cfopts.password), cfgfile);
  cfopts.authactive=ini_getbool("auth", "active",CTLD_AUTHACTIVE, cfgfile);
  ini_gets("control", "owparms", CTLD_OWPARMS, cfopts.owparms, sizearray(cfopts.owparms), cfgfile);
  ini_gets("control", "sensor", CTLD_SENSORID, cfopts.sensor, sizearray(cfopts.sensor), cfgfile);
  
  /* get actuator options heating/cooling and stirring devices */
  for (i=0;i<2;i++) {
    if (0==i) {
      ini_gets("control", "actuator", CTLD_ACTUATORID, buf, sizearray(buf), cfgfile);
    } else {
      ini_gets("control", "stirring_device", CTLD_STIRRINGID, buf, sizearray(buf), cfgfile);
      if (strlen(buf)==0) cfopts.stirring=0; else cfopts.stirring=1;
    }
    strncpy(cfopts.actuator[i],buf,100);
    cfopts.actuator[i][99]='\0';
  }
  
  /* get additional options for stirring */
  if (cfopts.stirring) {
    int res;
    ini_gets("control", "stirring_states", CTLD_STIRSTATES, buf, sizearray(buf), cfgfile);
    cfopts.stirring_states[9][0]=0;
    cfopts.stirring_states[9][1]=0;
    res=sscanf(buf,"%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d",
      &cfopts.stirring_states[0][0], &cfopts.stirring_states[0][1],
      &cfopts.stirring_states[1][0], &cfopts.stirring_states[1][1],
      &cfopts.stirring_states[2][0], &cfopts.stirring_states[2][1],
      &cfopts.stirring_states[3][0], &cfopts.stirring_states[3][1],
      &cfopts.stirring_states[4][0], &cfopts.stirring_states[4][1],
      &cfopts.stirring_states[5][0], &cfopts.stirring_states[5][1],
      &cfopts.stirring_states[6][0], &cfopts.stirring_states[6][1],
      &cfopts.stirring_states[7][0], &cfopts.stirring_states[7][1],
      &cfopts.stirring_states[8][0], &cfopts.stirring_states[8][1]);
      
    if (res!=18) die("invalid syntax of option stirring_states, needs to be exactly 9: %s\n",buf);
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
  ini_gets("global", "plugin_dir", CTLD_PLUGINDIR, cfopts.plugindir, sizearray(cfopts.plugindir), cfgfile);

  // read rest times and temperatures
  for (i=0;i<3;i++) {
    cfresttemp[8]='1'+i;
    cfopts.resttemp[i]=ini_getf("mash-process",cfresttemp,defrtemp[i], cfgfile);
    cfresttime[8]='1'+i;
    cfopts.resttime[i]=ini_getl("mash-process",cfresttime,defrtime[i], cfgfile);
  }
  cfopts.resttemp[i]=ini_getf("mash-process","lauteringtemp",defrtemp[i], cfgfile);
  cfopts.resttime[i]=ini_getl("mash-process","lauteringtime",defrtime[i], cfgfile);

  ini_gets("mash-process", "state_change_cmd", "", cfopts.state_change_cmd,
	     sizearray(cfopts.state_change_cmd), cfgfile);
  ini_gets("global", "conf_change_script", "", cfopts.conf_change_script,
	     sizearray(cfopts.conf_change_script), cfgfile);
}
