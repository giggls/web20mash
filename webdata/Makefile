JQUERY_BASE=http://code.jquery.com
JQUERY=jquery-1.8.2.min.js

JCANVASCRIPT_BASE=http://jcscript.com/data/js
JCANVASCRIPT=jCanvaScript.1.5.18.min.js

JSTEPPER_BASE=http://jstepper.emkay.dk/scripts
JSTEPPER=jquery.jstepper.min.js

SPINNER_BASE=http://fgnass.github.com/spin.js/dist/
SPINNER=spin.min.js

HTML_IN_FILES := $(wildcard *.html.*.in)
HTML_OUT_FILES := $(patsubst %.in, %, ${HTML_IN_FILES})

all: images/thermometer.png images/sudpfanne.png images/propeller1.png js/$(JQUERY) js/$(JCANVASCRIPT) js/$(JSTEPPER) images/en.png ${HTML_OUT_FILES} js/$(SPINNER) 

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
	
images/propeller1.png: images/propeller_anim.svg
	inkscape images/propeller_anim.svg -D -w 50 -i layer1 -j --export-png=images/propeller1.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer2 -j --export-png=images/propeller2.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer3 -j --export-png=images/propeller3.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer4 -j --export-png=images/propeller4.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer5 -j --export-png=images/propeller5.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer6 -j --export-png=images/propeller6.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer7 -j --export-png=images/propeller7.png
	inkscape images/propeller_anim.svg -D -w 50 -i layer8 -j --export-png=images/propeller8.png
	
	
js/$(JQUERY):
	wget -P js $(JQUERY_BASE)/$(JQUERY)
	
js/$(JCANVASCRIPT):
	wget -P js $(JCANVASCRIPT_BASE)/$(JCANVASCRIPT)
	
js/$(JSTEPPER):
	wget -P js $(JSTEPPER_BASE)/$(JSTEPPER)

js/$(SPINNER):
	wget -P js $(SPINNER_BASE)/$(SPINNER)

images/en.png:
	wget -O - -q http://qt.osdn.org.ua/kbflags-1.0.tar.gz |tar xvz -C /tmp
	cp /tmp/kbflags-1.0/21x14.new/* images
	cp images/gb.png images/en.png
	rm -rf /tmp/kbflags-1.0
			
clean:
	rm -f images/thermometer.png
	rm -f images/sudpfanne.png
	rm -f images/propeller?.png
	rm -f js/$(JQUERY)
	rm -f js/$(JCANVASCRIPT)
	rm -f index.html.??
	