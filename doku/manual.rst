=================
Rhea Player Setup
=================
piano player robot standard Interface
-------------------------------------

RheaPlayer is an OS-independent Puredata application.

Quickstart::
 Open the main-patch 'RheaPlayer.pd' with Pd >=0.50 with internal puredata library pdpp and pde and external puredata libraries iemlib, zexy and cyclone pd libraries installed.


1. Prerequisites
-----------------

- Puredata version >= 0.50 (see http://msp.ucsd.edu/software.html )
  
  preferred Pd Vanilla Version >=0.50, but others should work also, if adjusting pathes, etc., too but are untestet

- Pd libraries must have:

  Get them via 'deken' over 'find externals' in help menu and put them in your favorite Pd-Library Path 
  or in the 'libs/' directory of the Rheaplayer.

  - iemlib (iemlib1 + iemlib2 + abstractions in iemlib)

  - zexy

- Pd libraries optional:

  - cyclone : seq object for midifile player instead of xeq (see notes on xeq below) 

  - acre : for data storage functionality

2. Configuration of Pd
-----------------------

If MIDI is used, configure the "Media->MIDI-Settings" in Pd and store settings (see Pd manual). 
No other setting is needed.

Note on Audio: 
    When Audio is disabled (no ADC/DAC set), the real-time timer of the system is used for timing, which is on most computer more accurate than using the jitter of the audio interfaces for the time base. This should reduce latencies and jitter.

    If audio functionality for your application is needed, configure audio. If audio and "DSP on", the timing is done by the audio-driver, so the resolution depends on the audio-buffer and latency settings there. Mostly this does not do any harm. If no audio is used, the system timer, or realtime system-timer if available, of the computer is used for the timing. Since the RheaPlayer Pd-patch is computational lightweight, there should not be any noticeable jitter above 1~ms, which for a piano with a 10~ms attack time of the hammers is more than suitable, but ... it depends on the situation.

3. Operation
-------------

network setup
.............

Attach an Ethernet Cable to rhea. Note that rhea can not detect cross-over situations, so use a crossover Ethernet-cable if your host can also no cross-over detection, very unlikely nowadays, or use an Ethernet hub.

The robot player of the series "rhea" has fixed IP addresses. For connection the number of the actual rhea, which is written on the Rheas Ethernet port or nearby, has to be configured in the settings and will be stored in the 'config.txt' for next starts. 

Configure your computers Ethernet interface with a fixed IP address in "manual" mode, choose one IP in the range of "192.168.10.1-192.168.10.127" and net mask 255.255.255.0 (example: 192.168.10.2/24). If more computer on the same network are attached, use different IP numbers for them.

The IP of the rhea-player is '192.168.10.(128+rheanumber)' and with a reinit the connection should be activated.

OSC functionality
.................

For playing the pianoplayer with open sound protocol (OSC), the UDP protocol is used. There are two ways to address the pianoplayer via the Rheaplayer Interface: as indexed piano enabling different pianos to be controlled over the same OSC connection and indirect with unaddressed MIDI notes:

indexed::

  /piano/<nr>/note <note> <velocity> ... <nr> = number of piano, <note> = MIDI note number, <velocity> = MIDI velocity or noteoff if 0
  
indirect::

 /midi/note <note> <velocity> ... <note> = MIDI note number, <velocity> = MIDI velocity or note off if 0

Also the "allNotesOff" message is supported.
  
MIDI Input
..........

Use the internal MIDI-Patch mechanism of your OS, if available. On Linux for example jack-midi, on Mac OS-X core MIDI settings with IAC-bus. If using an external MIDI-device on your computer route it in the Pd and turn on MIDI on the GUI of the patch.

4. Usage of the PD pianoplayer library
---------------------------------------

This Pd-Patch as application is heavily based on the pianoplayer library, which was extracted from the pianoplayer projects over the last centuries as common usage base. You can build your own Pd-application for your artwork using the libraries "pdpp" and "pde".

see libraries for more Information there.

Background Information on RheaPlayer
------------------------------------

For Open Hard- and Software Information see http://github.com/algorythmics/pianplayer

Notes on Xeq
------------

Xeq is a very nicely structured, efficient sequencer library for Pd, to be used for multi-tracking, loops, score following and
MIDI recording, originally written by Czaja [xeq] 
But the development of xeq has stopped. So a fork of it is targeted. In the meantime  seq of the cyclone library is used, which pops up a dialog if a MIDI-file is not found and therefore not usable for installations and has some other drawbacks.

orig: 
 http://suita.chopin.edu.pl/~czaja/miXed/externs/xeq.html
forked: 
 https://github.com/pd-externals/xeq


.. [pp] https://github.com/algorythmics/pianoplayer
 
.. [Pd] http://msp.ucsd.edu/software.html

.. [Rheaplayer] https://github.com/algorythmics/RheaPlayer

.. [pdpp] https://github.com/algorythmics/pdpp

.. [pde] https://github.com/algorythmics/pde

.. [xeq] http://suita.chopin.edu.pl/~czaja/miXed/externs/xeq.html
 
Information
-----------
:Author: Winfried Ritsch
:Contact: ritsch _at_ algo.mur.at, ritsch _at_ iem.at
:Copyright: GPL-v3: winfried ritsch -  algorythmics 2004+
:Version: see ../readme.rst
:Master: https://github.com/algorythmics/RheaPlayer - doku
