@echo off

REM -------------- Editiere Parameter unterhalb ---------

REM MIDIFILES INPUT und OUTPUT
SET MIDIFILE=tests/midifiles/freud_pa.mid
SET OUTPUT=tests/midifiles/freud_pano.mid

REM REGEL (MODE) einstellen: 2 fuer Noten zusammenfuehren nach regel 2, 0 fuer Pausen kuerzen
REM REGEL (MODE) einstellen: 1 fuer Noten zusammenfuehren nach regel pre2, 0 fuer Pausen kuerzen

SET MODE=0

 REM Analysefenster in ms:

 SET FENSTERDAUER=72   

 REM fuer MODE 0 - pausen laengen kuerzen
  SET MINLEN=25
  SET MINPAUSE=47

REM --- INSTALLATION
SET MIDICONV=bin\midiconv
REM SET MIDICONV=midiconv

REM -------- UNTERHALB NICHT mehr editieren -------------
%MIDICONV% -ccp -x -m%MODE% -t%PAIRS% -s%VELDIFF% -r %FENSTERDAUER% -n %MINDIFF% -u %SCALEVEL% -p%MINPAUSE% -l%MINLEN% -o %OUTPUT% %MIDIFILE%
