#!/bin/sh
cd $(dirname $0)

# Note: not needed anymore, since now git subtrees
echo not needed anymore, since now git subtrees
exit 0

# 1) algorythmics libs on github
base_path=algorythmics
base_server=github.com
needed_libs="pde pdpp"

for lib in $needed_libs
do
    echo try get or update $lib:
    if  [ -d $lib  ]; then
        git -C $lib pull
    else
	server=$base_server
        echo git checkout $lib
        git clone git@${server}:${base_path}/${lib}.git
        if [ $? -ne 0 ]; then
            git clone https://${server}/${base_path}/${lib}.git
        fi
    fi
done

# 2) pd libs outside deken...
base_path=pd
base_server=git.iem.at
needed_libs="acre"

for lib in $needed_libs
do
    echo try get or update $lib:
    if  [ -d $lib  ]; then
        git -C $lib pull
    else
	server=$base_server
        echo git checkout $lib
        git clone git@${server}:${base_path}/${lib}.git
        if [ $? -ne 0 ]; then
            git clone https://${server}/${base_path}/${lib}.git
        fi
    fi
done
