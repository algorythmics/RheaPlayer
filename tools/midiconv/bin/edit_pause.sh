#!/bin/sh

if [ $# -lt 4 ]; then 
 echo usage: $0   \<pause\> \<len\> \<filein\> \<fileout\>
 echo see skript No Error checking !!!!
 exit -1
fi


skriptdir=$(dirname $0)


midiconv=${skriptdir}/midiconv


pause=$1
len=$2
file=$3
out=$4
log=edit_pause.log

echo convertig file $file with minimal pause $pause, minimal len $len to $out 
echo with removing out before


test -f out && echo rm -v $out

echo $midiconv -x -fq -q -o $out   -p $pause -l $len $file | tee -a $log
$midiconv -x -fq -q -o $out   -p $pause -l $len $file | tee -a $log
