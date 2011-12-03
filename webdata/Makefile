JQUERY_BASE=http://code.jquery.com
JQUERY=jquery-1.7.min.js

JCANVASCRIPT_BASE=http://jcscript.com/data/js
JCANVASCRIPT=jCanvaScript.1.5.10.min.js

JSTEPPER_BASE=http://jstepper.emkay.dk/scripts
JSTEPPER=jquery.jstepper.min.js

all: images/thermometer.png images/sudpfanne.png js/$(JQUERY) js/$(JCANVASCRIPT) js/$(JSTEPPER)

images/thermometer.png: images/thermometer.xcf
	xcf2png -o images/thermometer.png images/thermometer.xcf thermometer legend

images/sudpfanne.png: images/sudpfanne.svg
	inkscape images/sudpfanne.svg -D -w 400 --export-png=images/sudpfanne.png
	
js/$(JQUERY):
	wget -P js $(JQUERY_BASE)/$(JQUERY)
	
js/$(JCANVASCRIPT):
	wget -P js $(JCANVASCRIPT_BASE)/$(JCANVASCRIPT)
	
js/$(JSTEPPER):
	wget -P js $(JSTEPPER_BASE)/$(JSTEPPER)
	
clean:
	rm -f images/thermometer.png
	rm -f images/sudpfanne.png
	rm -f js/$(JQUERY)
	rm -f js/$(JCANVASCRIPT)
