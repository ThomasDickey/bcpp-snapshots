#!/bin/sh
# $Id: run_test.sh,v 6.3 2004/06/20 23:13:23 tom Exp $
# Run a test to show that CONFLICT is working
BIN=`pwd`
PROG=$BIN/conflict
PATH=".:$BIN:/bin"; export PATH
#
cat <<eof/
**
**	Set PATH = $PATH
**	This produces no conflict (unless /bin contains conflict!):
eof/
$PROG
#
cat <<eof/
**
**	Set PATH = $PATH
**	This shows conflict between different filetypes
eof/
$PROG -r -t.c.o. conflict
cat <<eof/
**
**	Set PATH = $PATH
**	This repeats the last test, with pathnames-only
eof/
$PROG -p -r -t.c.o. conflict

# make a temporary directory
DIR=`mktemp -d ${TMPDIR-/tmp}/conflict.XXXXXX 2>/dev/null`
if test -z "$DIR" ; then
	DIR=/tmp/conflict.$$
	mkdir $DIR 	
fi
trap "cd /;rm -rf $DIR" 0 1 2 5 15
cd $DIR || exit 1

cat <<eof/
**
**	Add a dummy executable in the current directory
**		$DIR
**
**	producing a conflict with the executable
**		$PROG
**
**	For Unix, the conflict will be in the empty suffix (first column
**	of the report).  For Win32 it will be in the combination of empty
**	and "e" (.exe) suffixes.
eof/

# try to test conflicts on case-insensitive systems
TEST=conflict
rm -f $TEST
touch $TEST
if test -f Conflict ; then
	TEST=Conflict
	rm -f $TEST
fi

echo "#!${SHELL-/bin/sh}" >$TEST
echo echo hello >>$TEST
chmod 755 $TEST
$PROG -t..exe
