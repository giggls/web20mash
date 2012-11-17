/*

Implementation of an analog countdown timer on top of jCanvaScript

(c) 2011 Sven Geggus <sven-web20mash@geggus.net>

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


Member functions are "setvalue" and "setperiod"

create a new timer like this:

timer=new timerwidget('mycanvas',0,0,20);
timer=new timerwidget('mycanvas',0,0,20,100);

100 is the radius (default=80)
the width of the whole timer is 2*radius+10

*/

function timerwidget(canvas,x,y,period,radius) {
  var border=25;
  if (typeof radius == 'undefined' ) radius = 80;
  var width = 2*radius+10;
  var allticks=[];
  this.value=0;
    
  // calculate start of ticks and tick distance
  function getticks(hi,lo,phi) {
    if (!phi) phi=180;
     delta = hi-lo;
    if (phi >= 270) {
      delta *= 0.5;
    } else {
      if (phi <= 120) {
        delta *= 2;
      }
    }
  
    rho = Math.floor(Math.log(delta)/Math.log(10));
    d = delta*Math.pow(10, -1*rho);
  
    if (d < 2) {
      tic = 0.2;
    } else {
      if (d < 5) {
        tic = 0.5;
      } else {
        tic = 1;
      };
    };
      
    tic = tic*Math.pow(10, rho);
    firsttic = Math.floor(lo*Math.pow(10,-1*rho))*Math.pow(10,rho);
    while (firsttic < lo) firsttic += tic;
    return [firsttic,tic];
  }
    
  function genticks(period,x,y) {
    if (period > 0) {
      step=getticks(period,0,360)[1];
      // if stepwith > 1 shorten to integer value
      if (step >= 1) step = Math.round(step);
      for (t=step; t <= period; t+=step) {
        a = t*(360.0/period);
        y1 = Math.cos(a*3.1415/180.0)*(radius-30)-y;
        x1 = Math.sin(a*3.1415/180.0)*(radius-30)+x;
        y2 = Math.cos(a*3.1415/180.0)*(radius-20)-y;
        x2 = Math.sin(a*3.1415/180.0)*(radius-20)+x;
        y3 = Math.cos(a*3.1415/180.0)*(radius-10)-y;
        x3 = Math.sin(a*3.1415/180.0)*(radius-10)+x;
        line=jc.line([[(width/2.0)+x1,(width/2.0)-y1],[(width/2)+x2,(width/2)-y2]]);
        allticks.push(line);
        if (t == parseInt(t)) {
	  txt=t
        } else {
          txt=t.toFixed(1)
        }
        text=jc.text(txt,(width/2.0)+x3+1,(width/2.0)-y3+1).font('bold 14px Arial').align('center').baseline('middle');
        allticks.push(text);
      }
    }
  }

  jc.start(canvas);
  //jc.rect(x,y,width,width,"#00ff00",1).lineStyle({lineWidth:0});;
  jc.circle(width/2.0+x,width/2.0+y,radius,"#ffffff",1);
  jc.circle(width/2.0+x,width/2.0+y,radius+1,0).lineStyle({lineWidth:2, color:"#000000"});
  jc.circle(width/2.0+x,width/2.0+y,radius-border,"#cccccc",1);
  genticks(period,x,y);
  this.jc_slice=jc.Sector(x+width/2.0,y+width/2.0,-90,-90,radius-border,'#ff0000',1);
  jc.start(canvas);

  this.setvalue = function(val) {
    this.value=val;
    endAngle=360*(val/period)-90;
    this.jc_slice.attr('endAngle',endAngle);
    jc.start(canvas);
  }

  function delticks() {
    for (i in allticks) {
      allticks[i].del();
    }
   jc.start(canvas);
  }

  this.setperiod = function(p) {
    period=p;
    delticks();
    genticks(period,x,y);
    this.setvalue(this.value);
    jc.start(canvas);
  }
}
