// propeller animation widget
//
// call like this
// propanim=new Propeller("mycanvas",0,0,imagedir);
//
function Propeller(canvas,x,y,imgdir) {
  var propImgfiles = new Array();
  var propImgObjs = new Array();
  var numload=0;
  var curImage=0;
  var animate=0;
  var animimgObj;

  for (var i=1;i<9;i++) {
    propImgfiles.push("propeller"+i+".png")
  }
  for (var i=0;i<propImgfiles.length;i++) {
    propImgObjs[i]=new Image();
    propImgObjs[i].onload = function() {
      numload++;
      if (numload==propImgfiles.length) {
        animimgObj=jc.image(propImgObjs[curImage],x,y);
        jc.start(canvas);
      }
    }
    propImgObjs[i].src=imgdir+propImgfiles[i];
  };
 
  this.drawnext = function() {
    if (curImage==7) curImage=0; else curImage++;
    animimgObj.attr('img', propImgObjs[curImage]);
    jc.start(canvas);
    if (animate) setTimeout( this.drawnext.bind(this) , 100)
  }
  this.start = function() {
    animate=1;
    this.drawnext();
  }
  this.stop = function() {
    animate=0;
  }
};

