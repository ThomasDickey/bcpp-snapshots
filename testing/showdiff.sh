#!/bin/sh
# $Id: showdiff.sh,v 7.0 1990/08/30 07:54:55 ste_cm Rel $
#
# Performs regression-test comparison for 'run_tests.sh'
if test ! -f $2
then
	mv $1 $2
elif ( cmp -s $1 $2 )
then
	echo '**	(ok)'
else
	echo '??	failed:'
	diff $1 $2
fi
