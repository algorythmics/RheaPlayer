MIDICONV --

midi-file tester and converter for autoklavier,
the program reads midi-files, detect too short pauses or notes and outputs a corrected midi-file.
This is done in a more or less sophisticated way especially for compositions of Peter Ablinger.

common usage:

  It let you define the minimum pause and minimum length between notes of the same pitch


special usage:

  to eliminate notes and wight their velocity if analysis was to fast:


use: midivonv -h for actual command-line options


Known Limits:

- ASSUMPTION of not changing ticktime during track
  which is a limitation !!!
sq
winfried ritsch
V 0.8, (c) Algorythmics 2005
