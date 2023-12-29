#!/bin/sh
cd $(dirname $0)

# acre lib master
echo test for acre
if  [ ! -d acre  ]; then
 	echo no acre
 else
 	git -C acre status
fi

# algorythmics libs on github
base_path=algorythmics
base_server=github.com
needed_libs="pde pdpp"

for lib in $needed_libs
do
    echo status of $lib:
    if  [ -d $lib  ]; then
      git -C $lib status        
    else
      echo no $lib
    fi
done
