# Make file supplied by Piercarlo Grandi <piercarl@sabi.demon.co.uk>

D		=../code/

CXX		=gcc
CPPFLAGS	=-I$(D) -O2 -Wall #-DDEBUG -DDEBUG2
CXXFLAGS	= -x c++ $(CPPFLAGS)

.SUFFIXES:	.cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

BCPP.o	= \
        $(D)bcpp.o \
        $(D)anyobj.o \
        $(D)baseq.o \
        $(D)debug.o \
        $(D)execsql.o \
        $(D)hanging.o \
        $(D)stacklis.o \
        $(D)cmdline.o \
        $(D)config.o \
        $(D)strings.o \
        $(D)tabs.o

bcpp:		$(BCPP.o)
	$(CXX) $(BCPP.o) -o $@ -lg++

clean::
	rm -f *.o bcpp *~ *# *.bak

$(BCPP.o) : config.h

$(D)bcpp.o \
$(D)debug.o \
$(D)execsql.o \
$(D)hanging.o \
$(D)tabs.o : bcpp.h
