#######################################################
#  Makefile for Midi Processor for Autoklavier Files
#  Winfried Ritsch
#
#  Feb 1998 -
#
# CC:  gcc and mingw
#######################################################

NAME = midiconv

HEADERS =  midilib/mididefs.h midilib/midifile.h src/midiconv.h
DEPALL = makefile

DEBUG = -g
#DEBUG =

CFLAGS = -DMF_OLDNAMES -Wall -Imidilib -Lmidilib $(DEBUG)

LINCC=gcc
LINUXFLAGS = -fPIC $(CFLAGS)
WINFLAGS = -DNT $(CFLAGS)

WINCC=i686-w64-mingw32-gcc
#WINCC=i586-mingw32msvc-gcc

all:	$(PROGRAMS)

linux: bin/midiconv

win: bin/midiconv.exe

CONVSRCS = src/midiconv.c src/change.c src/simple_pair.c src/dynamic_pair.c \
           src/modify.c src/storage.c src/read.c midilib/midifile.c
CONVOS = $(CONVSRCS:.c=.o)
CONVOBJS = $(CONVSRCS:.c=.obj)

bin/midiconv: $(CONVOS)
	$(CC) $(LDFLAGS) -o $@ $(CONVOS) $(LIBS)

midiconv.o: midiconv.c  $(HEADERS) $(CONVSRCS) $(DEPALL)

bin/midiconv.exe: $(CONVOBJS)
	$(WINCC) $(WINFLAGS) -o bin/midiconv.exe -Imidilib/  $(CONVOBJS)

%.obj: %.c
		$(WINCC) $(WINFLAGS) -c -o $@ $<

midilib/midifile.c:
	./get_midilib.sh
		
midilib/midifile.obj: $(HEADERS) $(CONVSRCS) $(DEPALL)
	$(WINCC) $(WINFLAGS) -c -o $@  midilib/midifile.c


clean-linux:
	-rm  -v midilib/*.o src/*.o delme*

clean-win:
	-rm  -v midilib/*.obj src/*.obj delme*

clean: clean-linux clean-win

dist-clean: clean-linux clean-win
	-rm -v bin/midiconv  bin/midiconv.exe  *~

# ----------- TEST ------------

#testfile=tests/midifiles/freud_pa.mid
testfile=tests/midifiles/test_modus0.mid

run-linux: bin/midiconv
	bin/midiconv -cpp -x -y3 -m0 -o delme.mid -p 200 -l 200 $(testfile)

run-win:
	wine bin/midiconv.exe -h
# 	./midiconv -x -fq -q -o delme2.midi -p 20 -l 50 test2.mid
