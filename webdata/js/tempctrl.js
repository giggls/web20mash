/* 

tempctrl

a Web 2.0 two-level temperature controler

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

*/

var tCanvas='TempCanvas';
var timgfile = "thermometer.png";
var imagedir="images/";
var thermo;
var timg = new Image();
var settings = {musttemp: 0, mpstate: 42, ctrl: 42};
var first = true;
var getstate="./getstate";
var setmust="./setmust/";
var setctl="./setctl/";
var setactuator="./setactuator/0/";
var setacttype="./setacttype/";

timg.onload = function() {
  RunApp();
}

timg.src=imagedir+timgfile;


function AjaxError(action) {
  $("body").css('background-image', 'none');
  $("body").css('background-color', '#FF0000');
  alert(i18n.comm_error+action);
}

function cbsetmust(data) {
 if (!data) {
   AjaxError('cbsetmust');
 }
}

function cbsetctl(data) {
 if (!data) {
   AjaxError('cbsetctl');
 }
}

function cbsetactuator(data) {
 if (!data) {
   AjaxError('cbsetactuator');
 }
}

function cbsetacttype(data) {
 if (!data) {
   AjaxError('cbsetacttype');
 }
}

// This does all the initial stuff and is called after the image has been loaded
function RunApp() {
  if ( $.browser.msie ) {
    $.ajaxSetup({ cache: false });
  }
  // input entry checks
  $("input[name='musttemp']").jStepper({minValue:0, maxValue:99, normalStep:0.5, decimalSeparator:"."});

  jc.start(tCanvas);
  thermo=new thermometerwidget(tCanvas,0,0,timg);
  jc.start(tCanvas);
  
  // deactivate all inputs
  $(".tempctl").attr("disabled", "true");
  $("input[name='actuator']").attr("disabled", "true");

  $('#setmust').click(function() {
      url=setmust+$("input[name='musttemp']").val();
      $.get(url, cbsetmust);
  });

  $("input[name='control']").click(function() {
     if ($(this).is (':checked')) {
       url=setctl+'1';
     } else {
       url=setctl+'0';
     }
     $.get(url, cbsetctl);
  });

  $("input[name='actuator']").click(function() {
     if ($(this).is (':checked')) {
       url=setactuator+'1';
     } else {
       url=setactuator+'0';
     }
     $.get(url, cbsetactuator);
  });

  $("input[name='acttype']").click(function() {
    var state = $("input[name='acttype']:checked").val();
    if (state == 1) {
      url=setacttype+"heater";
    } else {
      url=setacttype+"cooler";
    }
    $.get(url, cbsetacttype);
  });

  $.getJSON(getstate,parse_getstate_Response);
};

function thermometerwidget(canvas,x,y,image) {
  // jc.rect(x,y,90,330,"#00ff00",1);
  // thermometer background needs to be 22x201
  jc.rect(x+34,y+18,22,201,'#ffffff',1);
  tscale=jc.rect(x+39,y+218,12,1,'#8e0000',1);

  jc.image(image,x+20,y+5);
  this.indicator = jc.circle(x+45,y+244,0,'rgba(0,0,0,0.3)',true);
  
  templabel=jc.text("--.- °C",x+45,y+300);
  templabel.font('bold 25px Arial');
  templabel.align('center');
  mustlabel=jc.text("(--.- °C)",x+45,y+320);
  mustlabel.font('bold 15px Arial');
  mustlabel.align('center');

  this.setvalue = function(temp,must) {
     tscale.attr({y: ((y+218)-2*temp),height: (1+2*temp)});
     templabel.string(temp.toFixed(1)+'°C');
     this.indicator.attr('radius',17);
     window.setTimeout("clrindictr();",300);
     mustlabel.string('('+must.toFixed(1)+'°C)');
     jc.start(canvas);
  }
}

function clrindictr() {
  thermo.indicator.attr('radius',0);
  jc.start(tCanvas);
}

function parse_getstate_Response(data) {
  if (!data) {
    AjaxError('parse_getstate_Response');
  }

  thermo.setvalue(data.curtemp,data.musttemp);
  jc.start(tCanvas);

  // update settings from response

  // control checkbox
  if (data.ctrl) {
    $('input[name=control]').attr('checked', true);
  } else {
    $('input[name=control]').attr('checked', false);
  }

  // actuator checkbox
  if (data.rstate[0]) {
    $('input[name=actuator]').attr('checked', true);
  } else {
    $('input[name=actuator]').attr('checked', false);
  }

  // cooler/heater
  if (data.acttype == "heater") {
    $("input[name='acttype'][value=1]").attr("checked","checked");
  } else {
    $("input[name='acttype'][value=0]").attr("checked","checked");
  }

  // on very first response set initial state of control elements
  if (first) {
    first=false;
    $("input[name='musttemp']").val(data.musttemp);

    if (data.mpstate == 0)
      $(".tempctl").removeAttr('disabled');
    else
      $(".tempctl").attr("disabled", "true");

    if (data.ctrl == 0)
      $("input[name='actuator']").removeAttr('disabled');
    else
      $("input[name='actuator']").attr("disabled", "true");
  }

  // called if mpstate changed to 0
  if (data.mpstate == 0) {
    if (settings.mpstate!=0) {
       $(".tempctl").removeAttr('disabled');
    }
  }

  if (data.mpstate != 0) {
    // called if musttemp changed
     if (data.musttemp != settings.musttemp)
       $("input[name='musttemp']").val(data.musttemp);

    // called if mpstate changed from 0 to something else
    if (settings.mpstate==0) {
       $(".tempctl").attr("disabled", "true");
    }
  }

  // called if ctrl changed to 0
  if (data.ctrl == 0) {
    if (settings.ctrl!=0) {
      $("input[name='actuator']").removeAttr('disabled');
    }    
  }

  // called if ctrl changed to 1
  if (data.ctrl == 1) {
    if (settings.ctrl!=1) {
      $("input[name='actuator']").attr("disabled", "true");
    }    
  }

  /* this is essentially and endless recursion */
  $.getJSON(getstate,parse_getstate_Response);
  settings.mpstate=data.mpstate;
  settings.ctrl=data.ctrl;
  settings.musttemp=data.musttemp;
}

