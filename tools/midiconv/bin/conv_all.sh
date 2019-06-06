#!/bin/sh
# get all midifiles from midi.org and put it in midi

if [ $DEBUG ]; then DEBUG="echo"; else DEBUG=" "; fi

if [ $# -lt 4 ]; then 
 echo usage: $0   \<dirin\> \<dirout\> \<minpause\> \<minlen\> \<pattern\>
 echo example: $0 org conv 40 25 *.MID
 echo see skript No Error checking !!!!
 exit -1
fi

pause=$3
len=$4
pattern=$5
dirin=$1
dirout=$2

skriptdir=$(dirname $0)
midiconv=${skriptdir}/midiconv

if [ ! -d $dirin ];then echo no indir found; exit -1; fi
if [ ! -d $dirout ];then echo no outdir found; exit -1; fi
if [ ! -x $midiconv ];then echo no midiconv executable found; exit -1; fi

for f in `find ${dirin} -name "${pattern}" -o -name "*.mid"`
do
  echo $f

  filename=${f#*/}
  dir=`dirname $filename`

  #test -d $dir || mkdir -v -p $dir
  $DEBUG test -f ${dirout}/$filename || rm -v ${dirout}/$filename
  $DEBUG $midiconv -x -fq -q -o ${dirout}/$filename  -p $pause -l $len $f
  
done





