pde
===

Puredata escher library of of Puredata abstractions for controlling "EscherFetOS" running on an 'escher' board controlling a escherFET interface driving the solenoids for e.g.: the pianoplayers

Background
----------

'escher' is a simple micro-controller board made by Atelier Algorythmics with a Microchip dsPIC33F128MC708 microcontroller or any other PIN-compatible PIC like some PIC32. Populated with Ethernet interface it offers 80+ I/O Pins. The purpose of the board is to interface different hardware boards like the 'escherFET' used in the pianoplayer series of Atelier Algorythmics.

The board can be interfaced via Ethernet over the ENC28J60 controller or serial
interfaces.

Prerequisites
-------------

The escher board has to be programmed with an compatible "EscherOS" firmware. see firmware directory of the device,
e.g.: http://github.com/algorythmics/pianoplayer.

Theory of operation:
--------------------

The 'escher' has a Ethernet Port with unique MAC address and an fixed assigned IP address (static at the moment). The boards broadcasts UDP over the local net on the fixed info PORT number <announce-port>  and announce themselves with IP and MAC number. Afterwards they can be addressed by these IP-numbers. Also statistics like CPU-usage are reported over the the announce port.

Parsing and construction of the pde-syntax is done by helper functions, so each command implemented in the EscherOS has a corresponded pde object. These are prepared with optional control interface (GUI), see help files for details.

Info
----

announce-port: 
 6001

:Author: Winfried Ritsch
:Contact: ritsch _at_ algo.mur.at, ritsch _at_ iem.at
:Copyright: GPL-v3: winfried ritsch -  algorythmics 2004+
:Version: 1.0
:Master: https://github.com/algorythmics/pde
