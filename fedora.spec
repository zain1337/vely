#SPDX-License-Identifier: EPL-2.0
#Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
#Licensed under Eclipse Public License - v 2.0. See LICENSE file.
#On the web https://vely.dev/ - this file is part of Vely framework.

Name:   vely    
Version:    19.0.0
Release:    1%{?dist}
Summary:    Framework for C programming language. Rapid development of web and command-line applications.
Vendor:     Dasoftver LLC

Group:        Development/Tools
License:    EPL-2.0
Source0: http://nowebsite/vely/pkg/%{name}-%{version}-%{release}.tar.gz


#Packages in runtime that are not in build time are tar, curl, openssl and 

%define build_requires make gcc openssl-devel libcurl-devel rpm-build createrepo rpmlint pcre2-devel
#due to a bug in building dependencies for opensuse, must have this to build a dependency statement via bash script
#suse_build_requires *must* be identical to build_requires - do *not* change its definition
%define suse_build_requires %build_requires
%define run_requires tar make gcc openssl-devel curl libcurl-devel pcre2-devel

#devel lib for mariadb
%if 0%{?el9} 
%define maria_devel mariadb-connector-c-devel
%else
%define maria_devel mariadb-devel
%endif

#python utils for selinux, only for rhel
%if 0%{?rhel} 
%define rhel_sel policycoreutils-python-utils
%endif

#requirements:
#sudo dnf -y install rpmdevtools dnf-utils
#sudo dnf builddep -y fedora.spec

#OPENSUSE:sudo zypper -n install rpmdevtools yum-utils 
#OPENSUSE:sudo yum-builddep -y fedora.spec (bug with rpmutils missing, however)
#OPENSUSE has no default policy: no SELINUX
#OPENSUSE:FastCGI is for cgi-fcgi utility
%if 0%{?suse_version} >= 1500
BuildRequires: %suse_build_requires libmariadb-devel gpg sshpass FastCGI-devel postgresql-devel sqlite3-devel
Requires:    %run_requires libmariadb-devel FastCGI FastCGI-devel postgresql-devel sqlite3-devel


%else
#MAGEIA:sudo dnf -y install rpmdevtools dnf-utils
#MAGEIA:sudo dnf builddep -y mageia.spec
#MAGEIA:fcgi is for cgi-fcgi utility
%if 0%{?mgaversion} >= 8
#SELINUX:policycoreutils policycoreutils-devel libselinux-utils
BuildRequires: %build_requires mariadb-devel rpm-sign gnupg libfcgi-devel postgresql-devel policycoreutils policycoreutils-devel libselinux-utils sqlite3-devel
Requires:    %run_requires mariadb-devel fcgi libfcgi-devel apache-mod_proxy libpq-devel policycoreutils policycoreutils-devel libselinux-utils sqlite3-devel


%else
#SELINUX:policycoreutils policycoreutils-devel libselinux-utils
#fcgi is for cgi-fcgi utility; sometimes it's packaged within fcgi-devel and sometimes not.
BuildRequires: %build_requires %{?maria_devel} rpm-sign gpg fcgi-devel libpq-devel policycoreutils policycoreutils-devel libselinux-utils sqlite-devel
Requires:    %run_requires %{?maria_devel} %{?rhel_sel} fcgi fcgi-devel libpq-devel policycoreutils policycoreutils-devel libselinux-utils sqlite-devel
%endif
%endif


BuildRoot: %{_tmppath}/%{name}-%{version}
%global debug_package %{nil}

%description 
Vely is a development platform for C programming language running on Web Servers, MariaDB and Linux.


%prep
%setup -q -n %{name}-%{version}

#For faster building on Fedora 38, you can define special_build to yes. In this case, a step prior to rpmbuild must
#actually make Vely in $HOME/vely directory and build it in $HOME/vely/build directory. Otherwise, do not use this flag.
%build
%if "%{special_build}" == "yes"
cp ~/vely/* . || true
%else
make clean
make 
%endif

%install
%if "%{special_build}" == "yes"
rm -rf %{buildroot}
mkdir -p %{buildroot}
cp -rf ~/vely/build/* "%{buildroot}" || true
%else
rm -rf %{buildroot}
make DESTDIR="%{buildroot}" VV_NO_SEL=1 install
%endif

#SELINUX
#This must be after make install above; otherwise /var/lib/vv wouldn't exist yet (it exists in fakeroot, but that won't work for SELINUX)
#changing context (vely.sel 1) is done again here after being done in make install, because when rpm is installed, make install works
#in a fakeroot env, and restorecon isn't really operating on /var/lib/vv; thus it is worthless for when sudo dnf install is done.
#But, when installing from source with sudo make install, i.e. without dnf, that works on real /var/lib/vv. Some redundancy but it's
#difficult to make it without.
%post
%if 0%{?suse_version}  == 0
"%{_libdir}"/vely/selinux/vely.sel  "%{_libdir}/vely/selinux" "%{_datadir}" "%{_bindir}"
exit 0
%endif
#SELINUXEND



%files
%license LICENSE
%dir /var/lib/vv/bld/
%dir %{_libdir}/vely/
%dir %{_includedir}/vely/
%dir %{_datadir}/vely/examples/
%{_includedir}/vely/vely.h
%{_includedir}/vely/vfcgi.h
%{_libdir}/vely/libvely.so
%{_libdir}/vely/libvelylite.so
%{_libdir}/vely/libfcgively.so
%{_libdir}/vely/libvelyfcli.so
%{_libdir}/vely/libvelyfsrv.so
%{_libdir}/vely/libvelydb.so
%{_libdir}/vely/libvelypg.so
%{_libdir}/vely/libvelymys.so
%{_libdir}/vely/libvelysec.so
%{_libdir}/vely/libvelytree.so
%{_libdir}/vely/libvelypcre2.so
%{_libdir}/vely/libvelypcre2glibc.so
%{_libdir}/vely/libvelycurl.so
%{_libdir}/vely/stub_mariadb.o
%{_libdir}/vely/stub_pcre2.o
%{_libdir}/vely/stub_curl.o
%{_libdir}/vely/stub_crypto.o
%{_libdir}/vely/stub_before.o
%{_libdir}/vely/stub_after.o
%{_libdir}/vely/stub_startup.o
%{_libdir}/vely/stub_gendb.o
%{_libdir}/vely/stub_postgres.o
%{_libdir}/vely/stub_sqlite.o
%{_libdir}/vely/stub_fcgi.o
%{_libdir}/vely/stub_tree.o
%{_libdir}/vely/vmakefile
%{_libdir}/vely/vdiag
%{_libdir}/vely/v.vim
%{_libdir}/vely/sys
%{_libdir}/vely/pcre2_version
%{_libdir}/vely/pcre2_libs
%{_libdir}/vely/v1
%{_libdir}/vely/CHANGELOG
%{_bindir}/vf
%{_bindir}/vv
%{_mandir}/man2/*.2v*
%{_datadir}/vely/examples/*.tar.gz
#SELINUX
#Always distribute source selinux policy files; support for pp distribution is shaky
%if 0%{?suse_version}  == 0
%dir %{_libdir}/vely/selinux
%{_libdir}/vely/selinux/vv.te
%{_libdir}/vely/selinux/vv.fc
%{_libdir}/vely/selinux/vely.te
%{_libdir}/vely/selinux/vely.sel
%endif
#SELINUXEND

%changelog
* Sun Mar 06 2022 Dasoftver <vely@vely.dev> - 2.0.0-1
- Added SELinux install/remove. 
* Sun Aug 08 2021 Dasoftver <vely@vely.dev> - 1.0.0-1
- Initial version. 
