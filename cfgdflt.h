/*

mashctld

a web-controllable two-level temperature and mash process    
controler for 1-wire sensor (DS18S20/DS18B20) and various actuators 

(c) 2011-2012 Sven Geggus <sven-web20mash@geggus.net>

This file contains the compiled in defaults.

Sll these values can be changed using the runtime configuration file
(/etc/mashctld.conf by default)

*/
#define CTLD_PORT 8888
#define CTLD_OWPARMS "-u"
#define CTLD_SENSORID ""

#define CTLD_ACTUATORID ""

#define CTLD_STIRRINGID ""
#define CTLD_STIRSTATES "0:0,1:0,180:60,1:0,180:60,1:0,180:60,1:0,180:60"

#define CTLD_TEMPMUST 0
#define CTLD_HYSTERESIS 0.2
#define CTLD_ACTTYPE "heater"
#ifndef CTLD_WEBROOT
#define CTLD_WEBROOT "/usr/local/share/web20mash/webdata/"
#endif
#ifndef CTLD_PLUGINDIR
#define CTLD_PLUGINDIR "/usr/local/lib/web20mash/plugins/"
#endif
#define CTLD_RESTTIMES {0,20,20,0};
#define CTLD_RESTTEMP {52.5,62.5,72.5,76.0};
#define CTLD_USERNAME "login"
#define CTLD_PASSWORD "secret"
#define CTLD_AUTHACTIVE 0
