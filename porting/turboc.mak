# $Id: turboc.mak,v 1.2 1995/05/21 00:09:18 tom Exp $
#
# Turbo C/C++ makefile for C line-counter
# (adapted from PRJ2MAK output)
.AUTODEPEND 

# Define MSDOS for MS-DOS compatibility.
DEFINES = MSDOS

#		*Translator Definitions*
CC = tcc +c_count.cfg
MODEL = s
TLIB = tlib
TLINK = tlink
LIBPATH = C:\TC\LIB
INCLUDEPATH = .;\tc\include

OBJECTS = \
	c_count.obj \
	getopt.obj \
	wildcard.obj

all: c_count.exe

c_count.man: c_count.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	erase *.$$$
	erase c_count.cfg
	erase *.out
	erase *.obj
	erase *.bak
	erase *.log
	erase c_count.exe

#		*Implicit Rules*
.c.obj:
	$(CC) -c {$< }

#		*Explicit Rules*
c_count.exe: c_count.cfg $(OBJECTS)
	$(TLINK) /v/x/c/L$(LIBPATH) @&&|
c0$(MODEL).obj+
c_count.obj +
getopt.obj +
wildcard.obj
c_count
		# no map file
emu.lib+
math$(MODEL).lib+
c$(MODEL).lib
|

#		*Compiler Configuration File*
c_count.cfg: turboc.mak
	copy &&|
-m$(MODEL)
-v
-vi-
-w-ret
-w-nci
-w-inl
-wpin
-wamb
-wamp
-w-par
-wasm
-wcln
-w-cpt
-wdef
-w-dup
-w-pia
-wsig
-wnod
-w-ill
-w-sus
-wstv
-wucp
-wuse
-w-ext
-w-ias
-w-ibc
-w-pre
-w-nst
-I$(INCLUDEPATH)
-L$(LIBPATH)
-D$(DEFINES);STDC_HEADERS=1
| c_count.cfg

c_count.obj : c_count.cfg system.h
wildcard.obj : c_count.cfg system.h
