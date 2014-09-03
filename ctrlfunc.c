/* 

mashctld

a web-controllable two-level temperature and mash process
controler for various sensors and actuators

(c) 2011-2014 Sven Geggus <sven-web20mash@geggus.net>

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

control functions

*/
#include "mashctld.h"

extern struct configopts cfopts;
extern struct processstate pstate;
extern char cfgfp[PATH_MAX + 1];
extern void (*plugin_setstate_call[2])(int devno, int state);

/* clig command line Parameters*/  
extern bool actuator_simul[2];

/* two level control function, assume control actuator to be always device number 0 */
int doTempControl() {
#if 1
  // http://hobbybrauer.de/modules.php?name=eBoard&file=viewthread&tid=8185
  
  // Wenn Istwert >= (Sollwert – (Gradient * k)) dann ausschalten!
  if (pstate.tempCurrent >= (pstate.tempMust - (pstate.gradient*cfopts.k))) {
    if (pstate.relay[0]==1) {
      setRelay(0,0);
    }
  }
  // Wenn Istwert <= (Sollwert – Hysterese – (Gradient * k)) dann einschalten!
  if (pstate.tempCurrent <= (pstate.tempMust - cfopts.hysteresis - (pstate.gradient*cfopts.k))) {
    if (pstate.relay[0]==0) {
      setRelay(0,1);
    }
  }
#endif
#if 0
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
#endif
  return(0);
}


void setRelay(int devno, int state) {
  if (!actuator_simul[devno])
    plugin_setstate_call[devno](devno,state);
  pstate.relay[devno]=state;
}
