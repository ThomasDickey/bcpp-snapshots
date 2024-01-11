Summary: bcpp - C(++) beautifier
%define AppProgram bcpp
%define AppVersion 20240111
# $Id: bcpp.spec,v 1.11 2024/01/11 09:06:44 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: MIT
Group: Applications/Development
URL: ftp://ftp.invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppVersion}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>

%description
bcpp indents C/C++ source programs, replacing tabs with spaces or the
reverse. Unlike indent, it does (by design) not attempt to wrap long
statements.

This version improves the parsing algorithm by marking the state of all
characters, recognizes a wider range of indention structures, and implements
a simple algorithm for indenting embedded SQL.

%prep

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n %{AppProgram}-%{AppVersion}

%build

CPPFLAGS="$CPPFLAGS -DBCPP_CONFIG_DIR=\"%{_sysconfdir}\"" \
INSTALL_PROGRAM='${INSTALL}' \
%configure \
 --program-prefix=b \
 --target %{_target_platform} \
 --prefix=%{_prefix} \
 --bindir=%{_bindir} \
 --datadir=%{_datadir} \
 --sysconfdir=%{_sysconfdir} \
 --libdir=%{_libdir} \
 --mandir=%{_mandir}

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}
install -m 644 code/bcpp.cfg $RPM_BUILD_ROOT%{_sysconfdir}

strip $RPM_BUILD_ROOT%{_bindir}/%{AppProgram}

%files
%defattr(-,root,root)
%{_bindir}/%{AppProgram}
%{_bindir}/cb++
%{_sysconfdir}/bcpp.cfg
%{_mandir}/man1/%{AppProgram}.*

%changelog
# each patch should add its ChangeLog entries here

* Thu Jan 11 2024 Thomas E. Dickey
- remove obsolete clean-section
- use hardening macros

* Sun Apr 01 2018 Thomas Dickey
- update ftp url, suppress debug package

* Sun Mar 18 2012 Thomas Dickey
- initial version
