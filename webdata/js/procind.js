// Process on/off indicator using spin.js
// 
// http://fgnass.github.com/spin.js
//
//
function procindicator (thediv) {

  this.opts = {
    lines: 12, // The number of lines to draw
    length: 12, // The length of each line
    width: 3, // The line thickness
    radius: 12, // The radius of the inner circle
    corners: 1, // Corner roundness (0..1)
    rotate: 0, // The rotation offset
    color: 'black', // #rgb or #rrggbb
    speed: 0, // Rounds per second
    trail: 60, // Afterglow percentage
    shadow: false, // Whether to render a shadow
    hwaccel: false, // Whether to use hardware acceleration
    className: 'spinner', // The CSS class to assign to the spinner
    zIndex: 2e9, // The z-index (defaults to 2000000000)
    top: 'auto', // Top position relative to parent in px
    left: 'auto' // Left position relative to parent in px
  };
  
  this.target = document.getElementById(thediv);
  this.spinner = new Spinner(this.opts).spin(this.target);
  
  this.cs = function (c,s) {
    this.spinner.stop();
    this.opts.color=c;
    this.opts.speed=s;
    this.spinner = new Spinner(this.opts).spin(this.target);
  };
};
