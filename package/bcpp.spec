Summary: bcpp - C(++) beautifier
%define AppProgram bcpp
%define AltProgram cb++
%define AppVersion 20240917
# $Id: bcpp.spec,v 1.14 2024/09/17 23:19:11 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: MIT
Group: Applications/Development
URL: https://invisible-island.net/%{AppProgram}
Source0: https://invisible-island.net/archives/%{AppProgram}/%{AppProgram}-%{AppVersion}.tgz

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
%{_bindir}/%{AltProgram}
%{_sysconfdir}/bcpp.cfg
%{_mandir}/man1/%{AppProgram}.*
%{_mandir}/man1/%{AltProgram}.*

%changelog
# each patch should add its ChangeLog entries here

* Tue Sep 17 2024 Thomas E. Dickey
- rpmlint

* Thu Jan 11 2024 Thomas E. Dickey
- remove obsolete clean-section
- use hardening macros

* Sun Apr 01 2018 Thomas Dickey
- update ftp url, suppress debug package

* Sun Mar 18 2012 Thomas Dickey
- initial version
