# Make file supplied by Piercarlo Grandi <piercarl@sabi.demon.co.uk>

D		=../code/

C++		=gcc
C++FLAGS	=-I$(D) -O2

.SUFFIXES:	.cpp .o

.cpp.o:
	$(C++) $(C++FLAGS) -x c++ -c $< -o $@

BCPP.o		=$(D)bcpp.o $(D)anyobj.o $(D)baseq.o \
		 $(D)stacklis.o $(D)cmdline.o $(D)config.o

bcpp:		$(BCPP.o)
	$(C++) $(BCPP.o) -o $@

clean::
	rm -f $(BCPP.o) bcpp *~ *# *.bak
