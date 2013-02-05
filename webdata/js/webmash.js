/* 

Webmash

a Web 2.0 mash process controler

(c) 2011-2013 Sven Geggus <sven-web20mash@geggus.net>

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

var mashCanvas='ControlCanvas';
var jc_heat_img;
var jc_peng_img;
var timer;
var cimageObjs = new Array();
var cimgfiles = new Array("tux-brew.png","tux-brew-sleep.png","heating-on.png","heating-off.png","thermometer.png","sudpfanne.png");
var imagedir="images/";
var thermo;
var mpstate=42;
var settings = {ok: false, open: false, musttemp: 0, resttime: Array(0, 0,  0, 0), resttemp: Array(0, 0, 0, 0)};
// number of loaded canvas images
var numload=0;
var getstate="./getstate";
var setmpstate="./setmpstate/";
var setallmash="./setallmash/";
var setactuator="./setactuator/";
var iodinealert=0;
var stirring_active=0;
var stirring_state=0;
var propeller;
var pi;
var control=0;

for (var i=0;i<cimgfiles.length;i++) {
  cimageObjs[i]=new Image();
  cimageObjs[i].onload = function() {
     numload++;
     if (numload==cimgfiles.length) {
       RunApp();
     }
   }
   cimageObjs[i].src=imagedir+cimgfiles[i];
}
            
function AjaxError(action) {
  $("body").css('background-image', 'none');
  $("body").css('background-color', '#FF0000');
  alert(i18n.comm_error+action);
}

// This does all the initial stuff and is called after all images have been loaded
function RunApp() {
  if ( $.browser.msie ) {
    $.ajaxSetup({ cache: false });
  }
  // input entry checks
  $(".tempinput").jStepper({minValue:0, maxValue:99, normalStep:0.5, decimalSeparator:"."});
  $(".timeinput").jStepper({minValue:0, maxValue:120, allowDecimals:false, decimalSeparator:"."});

  drawcanvas();
  pi=new procindicator('procind');

  $.getJSON(getstate,parse_getstate_Response);
  $('#start').click(function() {
    $("#settings-btn").attr("disabled", "true");
    var startstate= $("input[name='mashstate']:checked").val();
    // if state 7 is entered manually do not alert
    if (startstate==7) {
      iodinealert = 1;
      pi.cs("red",0.6);
    };
    url=setmpstate + startstate;
    $('#state'+startstate).css("background","red");
    $.get(url, cbstart);
  });
  $('#stop').click(function() {
    if ((mpstate > 0) && (mpstate < 9)) {
      $('#state'+$("input[name='mashstate']:checked").val()).css("background","");
      url=setmpstate + '0';
      $.get(url, cbstop);
      for (var i=1;i<9;i++) {
        $('#state'+i).css("background","");
      }
      iodinealert = 0;
      pi.cs('black',0);
    }
  });
  $('#settings-btn').click(function() {
    if (settings.ok) {
      settings.open=true;
      // fill in the current values from server
      $("input[name='musttemp']").val(settings.musttemp);
      $("input[name='resttime1']").val(settings.resttime[0]);
      $("input[name='resttime2']").val(settings.resttime[1]);
      $("input[name='resttime3']").val(settings.resttime[2]);
      $("input[name='resttime4']").val(settings.resttime[3]);
      $("input[name='resttemp1']").val(settings.resttemp[0]);
      $("input[name='resttemp2']").val(settings.resttemp[1]);
      $("input[name='resttemp3']").val(settings.resttemp[2]);
      $("input[name='resttemp4']").val(settings.resttemp[3]);

      $('#settings-frame').toggle();
      $("#settings-btn").attr("disabled", "true");
    }
  });
  $('#OK').click(function() {
    url=setallmash+$("input[name='resttemp1']").val()+"/";
    url+=$("input[name='resttime1']").val()+"/";
    url+=$("input[name='resttemp2']").val()+"/";
    url+=$("input[name='resttime2']").val()+"/";
    url+=$("input[name='resttemp3']").val()+"/";
    url+=$("input[name='resttime3']").val()+"/";
    url+=$("input[name='resttemp4']").val()+"/";
    url+=$("input[name='resttime4']").val();
    $.get(url, cbsettings);
  });
  $("input[name='actuator']").click(function() {
     if ($(this).is (':checked')) {
       url=setactuator+'0/1';
       $.get(url, function(data){cbsetactuator(data, 1,'actuator')});
     } else {
       url=setactuator+'0/0';
       $.get(url, function(data){cbsetactuator(data, 0,'actuator')});
     }
  });
  $("input[name='stirrer']").click(function() {
     if ($(this).is (':checked')) {
       url=setactuator+'1/1';
       $.get(url, function(data){cbsetactuator(data, 1,'stirrer')});
     } else {
       url=setactuator+'1/0';
       $.get(url, function(data){cbsetactuator(data, 0,'stirrer')});
     }
  });
};

function thermometerwidget(canvas,x,y,image) {
  // jc.rect(x,y,90,330,"#00ff00",1);
  // thermometer background needs to be 22x201
  jc.rect(x+34,y+18,22,201,'#ffffff',1);

  // blue
  state1=jc.rect(x+34,218,22,0,'#9be1fb',1);
  // green
  state2=jc.rect(x+34,218,22,0,'#99ff99',1);
  // yellow
  state3=jc.rect(x+34,218,22,0,'#ffff77',1);
  // orange
  state4=jc.rect(x+34,218,22,0,'#FFaa55',1);
  // red
  state5=jc.rect(x+34,218,22,0,'#ff7777',1);
  

  tscale=jc.rect(x+39,218,12,1,'#8e0000',1);

  jc.image(image,x+20,y+5);
  templabel=jc.text("--.- °C",x+45,y+300);
  templabel.font('bold 25px Arial');
  templabel.align('center');
  mustlabel=jc.text("(--.- °C)",x+45,y+320);
  mustlabel.font('bold 15px Arial');
  mustlabel.align('center');

  this.setresttemp = function(t1,t2,t3,t4) {
    state1.attr({y: ((y+218)-2*t1),height: (1+2*t1)});
    state2.attr({y: ((y+218)-2*t2),height: (2*(t2-t1))});
    state3.attr({y: ((y+218)-2*t3),height: (2*(t3-t2))});
    state4.attr({y: ((y+218)-2*t4),height: (2*(t4-t3))});
    state5.attr({y: ((y+218)-200),height: (2*(100-t4))});
    jc.start(canvas);
  }
  this.clrresttemp = function() {
    state1.attr({height: 0});
    state2.attr({height: 0});
    state3.attr({height: 0});
    state4.attr({height: 0});
    state5.attr({height: 0});
    jc.start(canvas);
  }
  this.setvalue = function(temp,must) {
     tscale.attr({y: ((y+218)-2*temp),height: (1+2*temp)});
     templabel.string(temp.toFixed(1)+'°C');
     mustlabel.string('('+must.toFixed(1)+'°C)');
     jc.start(canvas);
  }
}

function drawcanvas() {
  jc.start(mashCanvas);
  jc.image(cimageObjs[5],40,20);
   
  thermo=new thermometerwidget(mashCanvas,195,50,cimageObjs[4]);
  jc_peng_img=jc.image(cimageObjs[0],60,290);
  jc_heat_img=jc.image(cimageObjs[3],330,350);
  
  timer=new timerwidget(mashCanvas,10,50,0);
  jc.start(mashCanvas);
}

function ptwinkle() {
  jc_peng_img.attr('img', cimageObjs[0]);
  jc.start(mashCanvas);
}

function parse_getstate_Response(data) {
  if (!data) {
    AjaxError('parse_getstate_Response');
  }

  settings.musttemp=data.musttemp;
  settings.resttime=data.resttime;
  settings.resttemp=data.resttemp;

  

  if (data.stirring) {
    if (!stirring_active) {
      $('#stirrer').show();
      propeller=new Propeller("ControlCanvas",350,290,"images/");
      stirring_active=1;
    } else {
      // edge detector for stirring state
      // turn propeller animation on or off
      if (stirring_state != data.rstate[1]) {
        stirring_state = data.rstate[1];
        if (stirring_state==1) {
          propeller.start();
        } else {
          propeller.stop();
        }
      }
    }
  }

  thermo.setvalue(data.curtemp,data.musttemp);
  if (data.rstate[0]) {
    jc_heat_img.attr('img', cimageObjs[2]);
  } else {
    jc_heat_img.attr('img', cimageObjs[3]);
  }

  // edge detection for change of control state on/off
  if (control != data.ctrl) {
    control=data.ctrl;
    if (data.rstate[0])
      $("input[name='actuator']").attr('checked', true);
    else
      $("input[name='actuator']").attr('checked', false);
    if (data.rstate[1])
       $("input[name='stirrer']").attr('checked', true);
    else
       $("input[name='stirrer']").attr('checked', false);
    if (data.ctrl == 0) {
      $("input[name='actuator']").removeAttr("disabled");
      $("input[name='stirrer']").removeAttr("disabled");
    } else {
      $("input[name='actuator']").attr("disabled", true);
      $("input[name='stirrer']").attr("disabled", true);
    }
  }

  // iodine alert popup once at the beginning of mpstate 7
  // as some people stop here just cancel this
  // if lautering temp. > temp.of second rest
  if ((data.mpstate == 7) && (iodinealert == 0) && (data.resttemp[3] > data.resttemp[2])) {
    iodinealert = 1;
    if (data.ctrl == 0) {
        pi.cs("black",0);
	alert(i18n.iodinealert);
    } else {
        pi.cs("red",0.6);
    }
  }

  // http://www.protofunc.com/scripts/jquery/checkbox-radiobutton/
  if (data.mpstate == 9) {
    // radiobutton back to start
    $("input[name='mashstate'][value=1]").attr("checked","checked");
    timer.setperiod(0);
    thermo.clrresttemp();
    $('#state6').css("background","");
    $('#state8').css("background","");
    iodinealert = 0;
    pi.cs('black',0);
    alert(i18n.process_finished);
  }
  
  if (data.mpstate == 0) {
    settings.ok=true;
    if (mpstate!=0) {
      // radiobutton back to start
      $("input[name='mashstate'][value=1]").attr("checked","checked");
      // clear state indicator (red background)
      $('#state'+mpstate).css("background","");
      // settings enabled
      $("#settings-btn").removeAttr('disabled');
      timer.setperiod(0);
      thermo.clrresttemp();
      iodinealert=0;
      pi.cs('black',0);
    }
  } else {
    // this is for opera only because Opera does not seem to honor
    // that our button has been disabled
    settings.ok=false;
  }
  
  if ((data.mpstate > 0) && (data.mpstate < 9)) {
    // this condition is met when new state is entered
    if (mpstate != data.mpstate) {
      restno=(data.mpstate/2)-1;
      thermo.setresttemp(data.resttemp[0],data.resttemp[1],data.resttemp[2],data.resttemp[3]);
      // this condition is met when rest is entered
      if (0 == (data.mpstate % 2)) {
        pi.cs("black",0.2);
        timer.setperiod(data.resttime[restno]);
      } else {
        timer.setperiod(0);
        if (data.mpstate != 7) pi.cs("red",0.6);
      }
      $("input[name='mashstate'][value="+data.mpstate+"]").attr("checked","checked");
      for (var j=1;j<data.mpstate;j++) {
        $('#state'+j).css("background","");
      }
      $('#state'+data.mpstate).css("background","red");
    }
    if (0 != data.resttimer) timer.setvalue(data.resttime[restno]-data.resttimer);
  }
  
  jc_peng_img.attr('img', cimageObjs[1]);
  jc.start(mashCanvas);
  window.setTimeout("ptwinkle();",300);
  /* this is essentially and endless recursion */
  $.getJSON(getstate,parse_getstate_Response);
  mpstate=data.mpstate;
}

function cbstart(data) {
 if (!data) {
   AjaxError('cbstart');
 }
}

function cbsettings(data) {
 if (!data) {
   AjaxError('cbsettings');
 } else {
    settings.open=false;
    settings.ok=false;
    $('#settings-frame').toggle();
    $("#settings-btn").removeAttr('disabled');
 }
}

function cbstop(data) {
 if (!data) {
   AjaxError('cbstop');
 } else {
   alert(i18n.process_canceled);
   $("#settings-btn").removeAttr('disabled');
 }
}

function cbsetactuator(data,state,name) {
 if (!data) {
   AjaxError('cbsetactuator');
 }
 if (name=='actuator') {
   if (state) {
     jc_heat_img.attr('img', cimageObjs[2]);
   } else {
     jc_heat_img.attr('img', cimageObjs[3]);
   }
  } else {
    stirring_state=state;
    if (state) {
      propeller.start();
    } else {
      propeller.stop();
    }
  }
  jc.start(mashCanvas);
}
