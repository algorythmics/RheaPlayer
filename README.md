pdpp
====
Pd Piano Player Library
-----------------------

Pd abstractions and presets for playing different piano player of the Atelier Algorythmics series like rhea.

see example and abstraction for help

settings and presets
--------------------

Data for different piano player numbered from 1..N, where the first rhea series is 1..12 . They can be overwritten with local calibration data files in own patches.

notemaps
   mapping of key numbers for eschers to MIDI-Note numbers

holds
 default hold value to provide a secure hold.

minmaxs
 maximum and minimum velocities. Note that these can be adjusted for each piano different should be done like this.
 
veltabs
 mapping MIDI velocity to key velocities.
 
 
 theory of operation
-------------------

receive a "midi note" as notenumber and a velocity as float, send it through an "noteeq" to a "pianostack", which holds all stages from note to key mapping until calibration and velocity mapping and sends it via the "pde" lib to the piano player.

most functionalities has an GUI named with '<func>_ctl.pd'. Variable names aka 'send/receives' are named in an OSC like notation.
Varialbles controlling a piano start with /piano/.. other with the interface names. See in the _ctl.pd patches which variables are exposed.

functionalities are grouped in sub-folders listed below:

data
  initial data for the pianoplayers

examples
  example patches and help patches for using pdpp

hold
  controlling the "hold" phase after the each attack

iface
  interfaces 

minmax
  handling and edit calibration for minimal and maximal velocities for each note

noteeq
  a halftone-filter eq, controlling the velocity distribution
  
np
  note processing functions 
  
rhea
  controlling a pianoplayer of the series rhea 
  
test
  test functionalities

veltab
  velocities mapping

Notes
-----

Note 1: with acre/ds datastorage for pianos can be used.


(C) GPL-v3.0 Winfried Ritsch, Atelier Algorythmics
