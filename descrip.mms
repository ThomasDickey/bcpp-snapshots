# $Id: descrip.mms,v 7.3 1995/05/21 23:05:07 tom Exp $
#
# VMS makefile for C_COUNT.  Requires "MMS"

TARGET = c_count.exe

LINKFLAGS = /MAP=$(MMS$TARGET_NAME)/CROSS_REFERENCE/EXEC=$(MMS$TARGET_NAME).EXE

INCS = []

SRC =	c_count.c \
	getopt.c \
	wildcard.c

OBJ =	c_count.obj,\
	getopt.obj,\
	wildcard.obj

all :
        @ decc = f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes.""
        @ axp = f$getsyi("HW_MODEL").ge.1024
        @ macro = ""
        @ if axp.or.decc then macro = "/MACRO=("
        @ if decc then macro = macro + "__DECC__=1,"
        @ if axp then macro = macro + "__ALPHA__=1,"
        @ if macro.nes."" then macro = f$extract(0,f$length(macro)-1,macro)+ ")"
        $(MMS)$(MMSQUALIFIERS)'macro' $(TARGET)

.IFDEF __ALPHA__
OPTFILE =
OPTIONS =
.ELSE
.IFDEF __DECC__
.ELSE
.ENDIF
OPTFILE = ,vmsshare.opt
OPTIONS = $(OPTFILE)/OPTIONS
.ENDIF

# install to DESTDIR1 if it's writable, else DESTDIR2
install :
	@ WRITE SYS$ERROR "** no rule for $@"
	
clean :
	@- if f$search("*.obj") .nes. "" then delete *.obj;*
	@- if f$search("*.bak") .nes. "" then delete *.bak;*
	@- if f$search("*.lis") .nes. "" then delete *.lis;*
	@- if f$search("*.log") .nes. "" then delete *.log;*
	@- if f$search("*.map") .nes. "" then delete *.map;*

distclean : clean
	@- if f$search("*.exe") .nes. "" then delete *.exe;*

check :
	set default [.testing]
	@run_test.dcl
	set default [-]	

$(OBJ) : system.h

.first :
	@ define/nolog SYS SYS$LIBRARY		! fix includes to <sys/...>

.last :
	@- if f$search("*.dia") .nes. "" then delete *.dia;*
	@- if f$search("*.lis") .nes. "" then purge *.lis
	@- if f$search("*.obj") .nes. "" then purge *.obj
	@- if f$search("*.map") .nes. "" then purge *.map
	@- if f$search("*.exe") .nes. "" then purge *.exe
	@- if f$search("*.log") .nes. "" then purge *.log

# used /G_FLOAT with vaxcrtlg/share in vms_link.opt
# can also use /Debug /Listing, /Show=All
CFLAGS =-
	/Diagnostics /Object=$@ /Include=($(INCS))

.C.OBJ :
	$(CC) $(CFLAGS) $(MMS$SOURCE)
	@- delete $(MMS$TARGET_NAME).dia;*

$(TARGET) : $(OBJ), descrip.mms $(OPTFILE)
	$(LINK) $(LINKFLAGS) $(OBJ) $(OPTIONS)

# Runs C_COUNT from the current directory (used for testing)
c_count.com :
	@- if "''f$search("$@")'" .nes. "" then delete $@;*
	@- copy nl: $@
	@ open/append  test_script $@
	@ write test_script "$ c_count :== $""sys$disk:''f$directory()'c_count.exe""
	@ write test_script "$ define/user_mode sys$input  sys$command"
	@ write test_script "$ define/user_mode sys$output sys$command"
	@ write test_script "$ c_count 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8"
	@ close test_script
	@ write sys$output "** made $@"
