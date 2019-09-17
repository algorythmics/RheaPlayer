pdpp
====
Pd Piano Player Library
-----------------------

Pd abstractions and presets for playing different robotic piano player of the Atelier Algorythmics, especially the series rhea but also others.

Tested Pd >= 0.48,  libraries pde, iemlib and zexy are needed (see find externals) 

For communication with rhea the pde library ( https://github.com/algorythmics/pde ) is used.
Older version of the pianoplayer like millitron and kantor have to use an other communication library using serial ports.

Robotic piano players see: http://algo.mur.at/projects/autoklavierspieler/


settings and presets
--------------------

Data for different piano player numbered from 1..N, where the first rhea series is 1..12 are included. 
They can be overwritten with local calibration data files in own patches.

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

Receive a "midi note" as note number and a velocity list, send it through an "noteeq" to a "pianostack", 
which process data from note to key mapping until calibration and velocity mapping and sends it via the "pde" lib to the piano player.

Most functionalities has an GUI named with '<func>_ctl.pd'. 
Variable names aka 'send/receives' are named in an OSC like notation.
Varialbles controlling a piano start with "/piano/.." the other with their interface names. 
See in the "*_ctl.pd" patches which variables as send/receives are exposed.

Functionalities are grouped in sub-folders listed below:

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
   note processing functions for manipulation notes

rhea
   controlling a pianoplayer of the series rhea 

test
   test functionalities

veltab
   velocities mapping

control
   simple MIDI-Controller and their mappings
   
vi
    simple visualisation in sync with video and text (needs Gem library)
   
Notes
-----

- with acre/ds datastorage for pianos can be used.

- noteprocessor handles "pianonotes" and "midinotes" aka "notes",
  where first contain the duration the other constructed with note ons and note offs.


:Author: Winfried Ritsch
:Contact: ritsch _at_ algo.mur.at, ritsch _at_ iem.at
:Copyright: GPL-v3: winfried ritsch -  algorythmics 2004+
:Version: 1.1
:Master: https://github.com/algorythmics/pdpp
