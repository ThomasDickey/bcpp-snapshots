@echo off
rem $Id: showdiff.bat,v 7.1 1995/05/21 00:21:20 tom Exp $
rem Compares files for C_COUNT sanity-tests
echo CASE %1 >>run_test.log
fc %2 %3 >>run_test.log
