Summary: C(++) beautifier
# $Id: bcpp.spec,v 1.15 2025/09/14 19:43:08 tom Exp $
Name: bcpp
Version: 20250914
Release: 1
License: MIT
Group: Applications/Development
URL: https://invisible-island.net/%{name}
Source0: https://invisible-island.net/archives/%{name}/%{name}-%{version}.tgz

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

%setup -q -n %{name}-%{version}

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

strip $RPM_BUILD_ROOT%{_bindir}/%{name}

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_bindir}/cb++
%{_sysconfdir}/bcpp.cfg
%{_mandir}/man1/%{name}.*
%{_mandir}/man1/cb++.*

%changelog
# each patch should add its ChangeLog entries here

* Sun Sep 14 2025 Thomas E. Dickey
- testing bcpp 20250914-1

* Tue Sep 17 2024 Thomas E. Dickey
- rpmlint

* Thu Jan 11 2024 Thomas E. Dickey
- remove obsolete clean-section
- use hardening macros

* Sun Apr 01 2018 Thomas Dickey
- update ftp url, suppress debug package

* Sun Mar 18 2012 Thomas Dickey
- initial version
