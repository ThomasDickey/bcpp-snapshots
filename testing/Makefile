# $Id: Makefile,v 7.2 1995/05/14 21:16:12 tom Exp $
# make-file for testing C_COUNT

####### (Standard Lists) #######################################################
SCRIPTS	=\
	run_test.sh	run_test.com\
	showdiff.sh

REF_FILES = \
	cat.ref \
	history.ref \
	list.ref \
	normal.ref \
	quotes.ref \
	table.ref \
	table_p.ref

TST_FILES = \
	test1.c \
	test2.c \
	test3.c

SOURCES	= Makefile descrip.mms README $(SCRIPTS) $(REF_FILES) $(TST_FILES)

####### (Standard Productions) #################################################
all:

clean:	
	-rm -f *.log
run_test:
	./run_test.sh 
