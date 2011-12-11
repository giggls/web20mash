JQUERY_BASE=http://code.jquery.com
JQUERY=jquery-1.7.min.js

JCANVASCRIPT_BASE=http://jcscript.com/data/js
JCANVASCRIPT=jCanvaScript.1.5.10.min.js

JSTEPPER_BASE=http://jstepper.emkay.dk/scripts
JSTEPPER=jquery.jstepper.min.js

all: images/thermometer.png images/sudpfanne.png js/$(JQUERY) js/$(JCANVASCRIPT) js/$(JSTEPPER) index.html.en images/en.png

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

# build all language index files from templates
index.html.en:
	./genindex.sh
	
# fetch all language files from http://qt.osdn.org.ua/kbflags-1.0.tar.gz
images/en.png:
	wget -O - -q http://qt.osdn.org.ua/kbflags-1.0.tar.gz |tar xvz -C /tmp
	cp /tmp/kbflags-1.0/21x14.new/* images
	cp images/gb.png images/en.png
	rm -rf /tmp/kbflags-1.0
			
clean:
	rm -f images/thermometer.png
	rm -f images/sudpfanne.png
	rm -f js/$(JQUERY)
	rm -f js/$(JCANVASCRIPT)
