dnl $Id: aclocal.m4,v 1.6 2005/04/10 16:24:23 tom Exp $
dnl autoconf macros for BCPP
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 3 updated: 1997/10/18 14:42:41
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 10 updated: 2003/04/17 22:27:11
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          display "compiling" commands],
	[
    ECHO_LT='--silent'
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LT=''
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 1 updated: 2004/12/03 20:27:48
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.
dnl It tries to imitate gcc, but does not return an error when it finds a
dnl mismatch between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC, to ensure that it is
dnl not mistaken for gcc.
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_PROG_CC])

INTEL_COMPILER=no

if test "$GCC" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel compiler)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[INTEL_COMPILER=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		CFLAGS="$cf_save_CFLAGS"
		AC_MSG_RESULT($INTEL_COMPILER)
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GXX_VERSION version: 3 updated: 2003/09/06 19:16:21
dnl --------------
dnl Check for version of g++
AC_DEFUN([CF_GXX_VERSION],[
AC_REQUIRE([AC_PROG_CPP])
GXX_VERSION=none
if test "$GXX" = yes; then
	AC_MSG_CHECKING(version of g++)
	GXX_VERSION="`${CXX-g++} --version|sed -e '2,$d'`"
	AC_MSG_RESULT($GXX_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GXX_WARNINGS version: 2 updated: 2005/04/02 13:49:56
dnl ---------------
dnl Check if the compiler supports useful warning options.
dnl
dnl Most of gcc's options apply to g++, except:
dnl	-Wbad-function-cast
dnl	-Wmissing-declarations
dnl	-Wnested-externs
dnl
dnl Omit a few (for now):
dnl	-Winline
dnl
dnl Parameter:
dnl	$1 is an optional list of g++ warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GXX_WARNINGS],
[
AC_REQUIRE([CF_INTEL_COMPILER])
AC_REQUIRE([CF_GXX_VERSION])

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion
	EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -Wall \
 -wd1419 \
 -wd193 \
 -wd279 \
 -wd593 \
 -wd810 \
 -wd869 \
 -wd981"
elif test "$GXX" = yes
then
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
	AC_CHECKING([for $CXX warning options])
	cf_save_CXXFLAGS="$CXXFLAGS"
	EXTRA_CXXFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wabi \
		fabi-version=0 \
		Woverloaded-virtual \
		Wsign-promo \
		Wsynth \
		Wold-style-cast \
		Weffc++ \
		Wcast-align \
		Wcast-qual \
		Wmissing-prototypes \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_warn_CONST $1
	do
		CXXFLAGS="$cf_save_CXXFLAGS $EXTRA_CXXFLAGS -Werror -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -$cf_opt"
		else
			test -n "$verbose" && AC_MSG_RESULT(... no -$cf_opt)
		fi
	done
	rm -f conftest*
	CXXFLAGS="$cf_save_CXXFLAGS"
fi
AC_LANG_RESTORE
AC_SUBST(EXTRA_CXXFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 2 updated: 1997/09/05 10:45:14
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
])dnl
