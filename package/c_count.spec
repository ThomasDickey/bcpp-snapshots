Summary: c_count - C-language line counter
%define AppProgram c_count
%define AppVersion 7.13
# $Id: c_count.spec,v 1.3 2013/08/24 19:24:03 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: MIT
Group: Applications/Development
URL: ftp://invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppVersion}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>

%description
C_count  counts  lines  and  statements in C-language source files.  It
provides related statistics on the amount of whitespace,  comments  and
code.   C_count  also  shows  the  presence  of  unbalanced (or nested)
comments, unbalanced quotation marks and illegal characters.

%prep

%setup -q -n %{AppProgram}-%{AppVersion}

%build

INSTALL_PROGRAM='${INSTALL}' \
	./configure \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--mandir=%{_mandir}

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install                    DESTDIR=$RPM_BUILD_ROOT

strip $RPM_BUILD_ROOT%{_bindir}/%{AppProgram}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/%{AppProgram}
%{_mandir}/man1/%{AppProgram}.*

%changelog
# each patch should add its ChangeLog entries here

* Fri Jul 16 2010 Thomas Dickey
- initial version
