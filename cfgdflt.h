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
#define CTLD_SENSORID "10.B963D4000800"
#define CTLD_ACTUATORID "12.9B6E45000000"
#define CTLD_EXTACTON "sispm +1"
#define CTLD_EXTACTOFF "sispm -1"
#define CTLD_TEMPMUST 0
#define CTLD_HYSTERESIS 0.2
#define CTLD_ACTTYPE "heater"
#define CTLD_WEBROOT "/usr/local/share/web20mash/webdata/"
#define CTLD_RESTTIMES {0,20,20,0};
#define CTLD_RESTTEMP {52.5,62.5,72.5,78.0};
#define CTLD_USERNAME "login"
#define CTLD_PASSWORD "secret"
#define CTLD_AUTHACTIVE 0

