========
MIDICONV
========
midi-file tester and converter for autoklavier
-----------------------------------------------

A command line tool to manipulate MIDI-Files, so it can be played on an Automatic Pianoplayer more acurate and near the limits.

MIDI-notes for my automatic piano player, does not play the piano keys, but the piano player, which means: finger down and up. The difference is, that a pause is needed for the key to move up again, apposed to synthesizer where ther is no physical constrain. Each piano has different speed of releasing keys, so we have to adjust the MIDI Input.

The program reads midi-files, detect too short pauses or notes and outputs a corrected midi-file. This is done in a more or less sophisticated way and has been developped especially for compositions of Peter Ablinger and other extreme performances.

common usage:

  Define the minimum pause and minimum length between notes of the same keys.

special usage:

  Eliminate notes and distribute their velocity if repetition of notes is too fast.

use: midivonv -h for actual command-line options

Known Limits:

- ASSUMPTION: No changing ticktime during track
  which is a limitation !!!

winfried ritsch
V 0.8, (c) Algorythmics 2005
