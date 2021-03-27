Summary: misc_tools - miscellaneous foundation tools
%define AppProgram misc_tools
%define AppVersion 20210327
# $XTermId: misc_tools.spec,v 1.20 2021/03/27 18:03:46 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: MIT
Group: Applications/Development
URL: ftp://ftp.invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppVersion}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>
Requires: sudo

%description
These are various programs which I use in my development environment,
and install on each machine, but which are too small to package separately.

%prep

%global my_bindir %{_libdir}/misc_tools

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n %{AppProgram}-%{AppVersion}

%build

INSTALL_PROGRAM='${INSTALL}' \
	./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{my_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--with-sudo-hacks=root,dickey

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_datadir}/%name
touch    $RPM_BUILD_ROOT%{_datadir}/%name/setuid

pushd $RPM_BUILD_ROOT%{my_bindir}
for prog in *
do
	case `file $prog` in
	*script*|*text*)
		;;
	*executable*)
		strip $prog
		;;
	esac
done
popd

%post

ln -v -sf %{my_bindir}/newpath %{_bindir}/

if [ -f %{_datadir}/%{name}/setuid ]
then
pushd %{my_bindir}
for prog in `cat %{_datadir}/%{name}/setuid`
do
	if id $prog >/dev/null 2>/dev/null
	then
		U=`id $prog | sed -e 's/^uid=//' -e 's/(.*//'`
		G=`id $prog | sed -e 's/^.*gid=//' -e 's/(.*//'`
		chown -v $U $prog
		chgrp -v $G $prog
		chmod -v ug+s $prog
	fi
done
popd
fi

%clean
if rm -rf $RPM_BUILD_ROOT; then
  echo OK
else
  find $RPM_BUILD_ROOT -type f | grep -F -v /.nfs && exit 1
fi
exit 0

%files
%defattr(-,root,root)
%{my_bindir}/*
%{_mandir}/man1/*
%{_datadir}/%name/*

%changelog
# each patch should add its ChangeLog entries here

* Sat Mar 27 2021 Thomas Dickey
- move binaries into lib-directory to work around name-pollution conflicts

* Sun Oct 25 2020 Thomas Dickey
- add manual pages

* Sat Mar 24 2018 Thomas Dickey
- update ftp url, disable debug package

* Tue Mar 13 2012 Thomas Dickey
- initial version
