// Sector Object extension for jCanvaScript
jc.addObject('Sector', { x: 0, y: 0, startAngle: 0, endAngle: 0, radius: 0, color: 'rgba(0,0,0,0)', fill: 0, r2: undefined },
      function (ctx) {
          var p1, p3, p4, startAngle = Math.PI * (this._startAngle / 180), endAngle = Math.PI * (this._endAngle / 180);
          ctx.save();
          p1 = { x: this._x + this._radius * Math.cos(startAngle), y: this._y + this._radius * Math.sin(startAngle) };
          if (this._r2) {
              p3 = { x: this._x + this._r2 * Math.cos(startAngle), y: this._y + this._r2 * Math.sin(startAngle) };
              p4 = { x: this._x + this._r2 * Math.cos(endAngle), y: this._y + this._r2 * Math.sin(endAngle) };
          }
          ctx.beginPath(); ctx.moveTo(this._x, this._y); if (this._r2) { ctx.moveTo(p3.x, p3.y); }
          ctx.lineTo(p1.x, p1.y);
          ctx.arc(this._x, this._y, this._radius, startAngle, endAngle, false);
          if (this._r2) {
              ctx.lineTo(p4.x, p4.y);
              ctx.arc(this._x, this._y, this._r2, endAngle, startAngle, true);
          } else {
              ctx.closePath();
          }
          ctx.fillStyle = this._color;
          ctx.strokeStyle = this._color;
          if (this._fill === false) { ctx.stroke(); } else { ctx.fill(); }
          ctx.restore()
      });
