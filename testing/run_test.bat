@echo off
rem $Id: run_test.bat,v 7.1 1995/05/21 00:21:48 tom Exp $
rem MSDOS script for exercising C_COUNT

if exist run_test.log  erase run_test.log

if exist count.in  erase count.in
if exist count.out erase count.out

rem
rem	Case 1:	Count lines in test-files (which have both unbalanced quotes and
rem		"illegal" characters:
rem
..\c_count test1.c test2.c >count.out
call .\showdiff 1 count.out normal.ref

rem
rem	Case 2:	Suppressing unbalanced-quote:
rem
..\c_count -qLEFT test1.c test2.c >count.out
call .\showdiff 2 count.out quotes.ref

rem
rem	Case 3:	Counting by names given in standard-input:
rem
echo test1.c >count.in
echo test2.c >>count.in
type count.in | ..\c_count >count.out
call .\showdiff 3 count.out list.ref

rem
rem	Case 4:	Counting bulk text piped in standard-input:
rem
type test1.c >count.in
type test2.c >>count.in
type count.in | ..\c_count -v - >count.out
call .\showdiff 4 count.out cat.ref

rem
rem	Case 5:	Counting history-comments
rem
..\c_count test3.c >count.out
call .\showdiff 5 count.out history.ref

rem
rem	Case 6:	Display as a spreadsheet
rem
..\c_count -t -qLEFT test1.c test2.c test3.c >count.out
call .\showdiff 6 count.out table.ref

rem
rem	Case 7:	Display as a spreadsheet (per-file)
rem
..\c_count -pt -qLEFT test1.c test2.c test3.c >count.out
call .\showdiff 7 count.out table_p.ref

if exist count.in  erase count.in
if exist count.out erase count.out

type run_test.log
