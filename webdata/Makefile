JQUERY_BASE=http://code.jquery.com
JQUERY=jquery-1.8.2.min.js

JCANVASCRIPT_BASE=http://jcscript.com/data/js
JCANVASCRIPT=jCanvaScript.1.5.18.min.js

JSTEPPER_BASE=http://jstepper.emkay.dk/scripts
JSTEPPER=jquery.jstepper.min.js

HTML_IN_FILES := $(wildcard *.html.*.in)
HTML_OUT_FILES := $(patsubst %.in, %, ${HTML_IN_FILES})

all: images/thermometer.png images/sudpfanne.png js/$(JQUERY) js/$(JCANVASCRIPT) js/$(JSTEPPER) images/en.png ${HTML_OUT_FILES}

index.html.%: index.html.%.in
	./html_from_template.sh index $*
	
mash.html.%: mash.html.%.in
	./html_from_template.sh mash $*
	
tempctrl.html.%: tempctrl.html.%.in
	./html_from_template.sh tempctrl $*

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
	rm index.html.??
	