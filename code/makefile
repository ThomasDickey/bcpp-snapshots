# Make file supplied by Piercarlo Grandi <piercarl@sabi.demon.co.uk>

D		=../code/

C++		=gcc
CPPFLAGS	=-I$(D) -O2 -Wall
C++FLAGS	= -x c++ $(CPPFLAGS)

.SUFFIXES:	.cpp .o

.cpp.o:
	$(C++) $(C++FLAGS) -c $< -o $@

BCPP.o	= \
        $(D)bcpp.o \
        $(D)anyobj.o \
        $(D)baseq.o \
        $(D)stacklis.o \
        $(D)cmdline.o \
        $(D)config.o \
        $(D)strings.o \
        $(D)tabs.o

bcpp:		$(BCPP.o)
	$(C++) $(BCPP.o) -o $@ -lg++

clean::
	rm -f $(BCPP.o) bcpp *~ *# *.bak

$(BCPP.o) : config.h

$(D)bcpp.o \
$(D)tabs.o : bcpp.h
