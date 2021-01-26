#!/bin/sh

# was midiconv, prepare your MIDI files
# now get this from github either by your methods or run this script
git clone git@github.com:algorythmics/ppmifis.git

if [ $? -ne 0 ]; then
	git clone https://github.com/algorythmics/ppmifis.git
fi
 
