@echo off

REM -------------- Editiere Parameter unterhalb ---------

REM MIDIFILES INPUT und OUTPUT
SET MIDIFILE=midi_convert\wachstum1_9ms.mid
SET OUTPUT=midi_convert\wachstum2_9u18ms_2.mid

REM REGEL (MODE) einstellen: 2 fuer Noten zusammenfuehren nach regel 2, 
REM REGEL (MODE) einstellen: 1 fuer Noten zusammenfuehren nach regel pre2
REM REGEL (MODE) einstellen: 0 fuer Pausen kuerzen

SET MODE=2

 REM wieviele noten maximal zusammenführen

 SET PAIRS=2 

 REM welche velocity differenz um neuen anschlag zu beginnen: -128 ... 128
 REM 0 heisst alle velos groesser wie vorhergehende neu anschlagen
 REM zB: -10 heisst alle die groesser 10 sind.

 SET VELDIFF=128

 REM Analysefenster in ms:

 SET FENSTERDAUER=9   

 REM fuer mode 1
  REM 1 maximum, 0 minimum, 0.5 mittelwert
  SET  SCALEVEL=1

 REM minimum vel difference for pause afterwards fuer mode 1
  set MINDIFF=-30

REM --- INSTALLATION
REM SET MIDICONV=C:\temp\midiconv

SET MIDICONV=midiconv

REM -------- UNTERHALB NICHT mehr editieren -------------
%MIDICONV% -ccp -x -m%MODE% -t%PAIRS% -s%VELDIFF% -r%FENSTERDAUER% -n%MINDIFF% -u%SCALEVEL% -o %OUTPUT% %MIDIFILE%
