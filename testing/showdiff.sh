#!/bin/sh
# $Id: showdiff.sh,v 7.1 2002/11/23 01:07:21 tom Exp $
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
	# show the reference on the left
	diff $2 $1 
fi
