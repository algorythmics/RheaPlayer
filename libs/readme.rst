Folder for internal and external puredata libraries.

Internal libraries
..................


pde
 Pd escher interface see: https://github.com/algorythmics/pde
 
pdpp
 Pd piano player library see: https://github.com/algorythmics/pdpp


Now included as subtrees, so no need to fetch it seprately,
(but be aware to commit them after work if needed seperatly)

to pull the newest version::

 git subtree pull --prefix=libs/pde https://github.com/algorythmics/pde.git master
 git subtree pull --prefix=libs/pdpp https://github.com/algorythmics/pdpp.git master

for developer commit inside::

 git -C libs/pde commit -m "..."
 git -C libs/pdpp commit -m "..."

 git subtree push --prefix=libs/pde https://github.com/algorythmics/pde.git master
 git subtree push --prefix=libs/pdpp https://github.com/algorythmics/pdpp.git master


The deprecated script get_libs.sh is placed in here to fetch or update the latest from github repos.


External libraries
..................

needed: iemlib, zexy
recommended: cyclone for midifiles

Note: Also deken can be used to download external libraries in here.

mfg
 winfried
