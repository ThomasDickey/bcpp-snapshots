$! $Header: /users/source/archives/c_count.vcs/porting/RCS/vmsbuild.com,v 1.2 1995/05/21 23:05:44 tom Exp $
$! VMS build-script for C_COUNT.  Requires "VAX-C" or "DEC-C".
$!
$
$ define/nolog SYS SYS$LIBRARY		! fix includes to <sys/...>
$
$! used /G_FLOAT with vaxcrtlg/share in vms_link.opt
$! can also use /Debug /Listing, /Show=All
$
$ axp  = f$getsyi("HW_MODEL").ge.1024
$ CC   = "CC"
$ DEFS = ""
$ OPTS = ""
$
$ if axp
$ then
$! assume we have DEC C
$	CFLAGS = "/standard=vaxc"
$	DEFS = ",HAVE_ALARM"
$ else
$! we have either VAX C or GNU C
$	CFLAGS = ""
$	DEFS = ",HAVE_SYS_ERRLIST"
$	OPTS = ",VMSSHARE.OPT/OPTIONS"
$	if (f$search("SYS$SYSTEM:VAXC.EXE").eqs."" .and. -
		f$trnlnm("GNU_CC").nes."") .or. (p1.eqs."GCC")
$	then
$		CC = "gcc"
$		OPTS = "''OPTS',GNU_CC:[000000]GCCLIB.OLB/LIB"
$	endif
$ endif
$
$ CFLAGS := 'CFLAGS/Diagnostics /Define=("os_chosen","''DEFS'") /Include=([])
$
$	if "''p1'" .nes. "" then goto 'p1
$
$
$ all :
$	call make c_count
$	call make getopt
$	call make wildcard
$
$	link /exec=c_count/map/cross c_count,getopt,wildcard, 'OPTS
$	goto build_last
$
$ install :
$	WRITE SYS$ERROR "** no rule for install"
$	goto build_last
$	
$ check :
$	set default [.testing]
$	@run_test.dcl
$	set default [-]	
$	exit
$	
$ distclean :
$	if f$search("*.exe") .nes. "" then delete *.exe;*
$
$ clean :
$	if f$search("*.bak") .nes. "" then delete *.bak;*
$	if f$search("*.lis") .nes. "" then delete *.lis;*
$	if f$search("*.log") .nes. "" then delete *.log;*
$	if f$search("*.map") .nes. "" then delete *.map;*
$	if f$search("*.obj") .nes. "" then delete *.obj;*
$	goto build_last
$
$ build_last :
$	if f$search("*.dia") .nes. "" then delete *.dia;*
$	if f$search("*.lis") .nes. "" then purge *.lis
$	if f$search("*.obj") .nes. "" then purge *.obj
$	if f$search("*.map") .nes. "" then purge *.map
$	if f$search("*.exe") .nes. "" then purge *.exe
$	if f$search("*.log") .nes. "" then purge *.log
$	exit
$
$ make: subroutine
$	if f$search("''p1'.obj") .eqs. ""
$	then
$		write sys$output "compiling ''p1'"
$		'CC 'CFLAGS 'p1
$		if f$search("''p1'.dia") .nes. "" then delete 'p1.dia;*
$	endif
$	exit
$ endsubroutine
