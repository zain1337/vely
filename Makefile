#SPDX-License-Identifier: EPL-2.0
#Copyright 2017 DaSoftver LLC. 
#Licensed under Eclipse Public License - v 2.0. See LICENSE file.
#On the web https://vely.dev/ - this file is part of Vely framework.

#makefile for Vely

.DEFAULT_GOAL=build
SHELL:=/bin/bash
#shortcut for DEBUGINFO
DEBUGINFO=$(DI)
#short cut for Address Sanitizer, internal only
ASAN=$(A)

#these must be the same (VV_PLATFORM_ID,VV_PLATFORM_VERSION) used in sys
OSNAME=$(shell . ./sys; echo -n $${VV_PLATFORM_ID})
OSVERSION=$(shell . ./sys; echo -n $${VV_PLATFORM_VERSION})
SYSTEMTYPE=$(shell ./sys showtype)
SYSTEMID=$(shell ./sys showid)
PGCONF=$(shell ./sys pgconf)
ANNOBIN=$(shell ./sys annobin)
DATE=$(shell date +"%b-%d-%Y")
VV_OS_CLOSE_2=$(shell grep OS_Close /usr/include/fcgios.h|grep shutdown|wc -l)

#these 2 must match application name from config file from each application

CC=gcc

#get build version and release
PACKAGE_VERSION=$(shell . .version; echo $${PACKAGE_VERSION})
BUILDVER=$(shell echo $(PACKAGE_VERSION)|sed "s/\(.*\)\.\(.*\)\.\(.*\)/\1.\2/g")
BUILDREL=$(shell echo $(PACKAGE_VERSION)|sed "s/\(.*\)\.\(.*\)\.\(.*\)/\3/g")


ifeq ($(strip $(BUILDVER)),)
BUILDVER=2
endif
ifeq ($(strip $(BUILDREL)),)
BUILDREL=100
endif

ifeq ($(strip $(SYSTEMTYPE)),fedora)
    V_LIB=$(shell rpm -E '%{_libdir}')/vely
else
    V_LIB=/usr/lib/vely
endif

ifeq ($(strip $(SYSTEMTYPE)),fedora)
    V_INC=$(shell rpm -E '%{_includedir}')/vely
else
    V_INC=/usr/include/vely
endif

ifeq ($(strip $(SYSTEMTYPE)),fedora)
    V_BIN=$(shell rpm -E '%{_bindir}')
else
    V_BIN=/usr/bin
endif

ifeq ($(strip $(SYSTEMTYPE)),fedora)
    V_MAN=$(shell rpm -E '%{_mandir}')/man2
else
    V_MAN=/usr/share/man/man2
endif
#see if man pages exist (or if not, need reindex)
MANEXIST=$(shell if [ -d "$(V_MAN)" ]; then echo "1"; else echo "0"; fi)


ifeq ($(strip $(SYSTEMTYPE)),fedora)
    V_VV_DATADIR=$(shell rpm -E '%{_datadir}')
    V_VV_DOCS=$(shell rpm -E '%{_datadir}')/vely
else
    V_VV_DATADIR=/usr/share
    V_VV_DOCS=/usr/share/vely
endif

ifeq ($(strip $(SYSTEMID)),opensuse)
    VV_FCGI_INCLUDE=-I /usr/include/fastcgi
else
    VV_FCGI_INCLUDE=
endif

V_VV_EXAMPLES=$(V_VV_DOCS)/examples

ifeq ($(strip $(PGCONF)),yes)
    VV_POSTGRES_INCLUDE=-I $(shell pg_config --includedir) 
else
    VV_POSTGRES_INCLUDE=$(shell  pkg-config --cflags libpq) 
endif

VV_MARIA_INCLUDE=$(shell mariadb_config --include)
VV_MARIA_LGPLCLIENT_LIBS=$(shell mariadb_config --libs) 
#$(shell mariadb_config --libs_sys)

#based on DEBUGINFO from debug file, we use appropriate tags
#Note: we always use -g in order to get line number of where the problem is
#(optimization is still valid though)
OPTIM_COMP_DEBUG=-g3 -DDEBUG -rdynamic
OPTIM_COMP_PROD=-O3 
OPTIM_LINK_PROD=
OPTIM_LINK_DEBUG=-rdynamic
ifeq ($(DEBUGINFO),1)
OPTIM_COMP=$(OPTIM_COMP_DEBUG)
OPTIM_LINK=$(OPTIM_LINK_DEBUG)
else
OPTIM_COMP=$(OPTIM_COMP_PROD)
OPTIM_LINK=$(OPTIM_LINK_PROD)
endif
ifeq ($(ASAN),1)
ASAN=-fsanitize=address -fsanitize-recover=address
else
ASAN=
endif

#C flags are as strict as we can do, in order to discover as many bugs as early on
CFLAGS=-std=gnu89 -Werror -Wall -Wextra -Wuninitialized -Wmissing-declarations -Wformat -Wno-format-zero-length -fpic $(VV_MARIA_INCLUDE) $(VV_POSTGRES_INCLUDE) $(VV_FCGI_INCLUDE)  -DVV_OSNAME="\"$(OSNAME)\"" -DVV_OSVERSION="\"$(OSVERSION)\"" -DVV_PKGVERSION="\"$(PACKAGE_VERSION)\"" $(OPTIM_COMP) $(ASAN)

#linker flags include mariadb (LGPL), crypto (OpenSSL, permissive license). This is for building object code that's part 
#this is for installation at customer's site where we link VELY with mariadb (LGPL), crypto (OpenSSL)
LDFLAGS=-Wl,-rpath=$(DESTDIR)$(V_LIB) -L$(DESTDIR)$(V_LIB) $(OPTIM_LINK) $(ASAN)

#Libraries and executables must be 0755 or the packager (RPM) will say they are not satisfied
.PHONY: install
install:
	install -m 0755 -d $(DESTDIR)/var/lib/vv/bld
	install -m 0755 -d $(DESTDIR)$(V_INC)
	install -D -m 0644 vely.h -t $(DESTDIR)$(V_INC)/
	install -D -m 0644 vfcgi.h -t $(DESTDIR)$(V_INC)/
	install -m 0755 -d $(DESTDIR)$(V_LIB)
	install -D -m 0755 libvelypg.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelydb.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelylite.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelymys.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelysec.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelycurl.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libfcgively.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvely.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelyfcli.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 libvelyfsrv.so -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_sqlite.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_postgres.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_mariadb.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_gendb.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_fcgi.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_curl.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_crypto.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_before.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_after.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 stub_startup.o -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 v.vim -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 vmakefile -t $(DESTDIR)$(V_LIB)/
	install -D -m 0644 CHANGELOG -t $(DESTDIR)$(V_LIB)/
	install -m 0755 -d $(DESTDIR)$(V_BIN)
	install -D -m 0755 v1 -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 vdiag -t $(DESTDIR)$(V_LIB)/
	install -D -m 0755 vv  -t $(DESTDIR)$(V_BIN)/
	install -D -m 0755 vf  -t $(DESTDIR)$(V_BIN)/
	install -m 0755 -d $(DESTDIR)$(V_MAN)
	install -D -m 0644 docs/*.2vv -t $(DESTDIR)$(V_MAN)/
	install -D -m 0755 sys -t $(DESTDIR)$(V_LIB)/
	sed -i "s|^[ ]*export[ ]*VV_LIBRARY_PATH[ ]*=.*|export VV_LIBRARY_PATH=$(V_LIB)|g" $(DESTDIR)$(V_LIB)/sys
	sed -i "s|^[ ]*export[ ]*VV_LIBRARY_PATH[ ]*=.*|export VV_LIBRARY_PATH=$(V_LIB)|g" $(DESTDIR)$(V_BIN)/vv
	sed -i "s|^[ ]*export[ ]*VV_INCLUDE_PATH[ ]*=.*|export VV_INCLUDE_PATH=$(V_INC)|g" $(DESTDIR)$(V_LIB)/sys
	sed -i "s|^[ ]*export[ ]*VV_VERSION[ ]*=.*|export VV_VERSION=$(BUILDVER).$(BUILDREL)|g" $(DESTDIR)$(V_LIB)/sys
	sed -i "s/\$$VERSION/$(BUILDVER).$(BUILDREL)/g" $(DESTDIR)$(V_MAN)/*.2vv
	sed -i "s/\$$DATE/$(DATE)/g" $(DESTDIR)$(V_MAN)/*.2vv
	for i in $$(ls $(DESTDIR)$(V_MAN)/*.2vv); do gzip -f $$i; done
	install -m 0755 -d $(DESTDIR)$(V_VV_DOCS)
	install -m 0755 -d $(DESTDIR)$(V_VV_EXAMPLES)
	install -D -m 0644 docs/velydocker.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/hash_server.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/cookies.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/create_table.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/form.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/sendmail.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/stock.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/file_manager.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/multitenant_SaaS.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/utility.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/hello_world.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/write_report.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/json.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
	install -D -m 0644 docs/shopping.tar.gz -t $(DESTDIR)$(V_VV_EXAMPLES)/
#This must be last, in this order, as it saves and then applies SELinux policy where applicable. 
#This runs during rpm creation or during sudo make install
#it does NOT run during rpm installation, there is post scriptlet that calls vely.sel to do that (VV_NO_SEL)
	if [[ "$(SYSTEMTYPE)" == "fedora" && "$(SYSTEMID)" != "opensuse" ]]; then install -D -m 0644 vv.te -t $(DESTDIR)$(V_LIB)/selinux ; install -D -m 0644 vely.te -t $(DESTDIR)$(V_LIB)/selinux ; install -D -m 0644 vv.fc -t $(DESTDIR)$(V_LIB)/selinux ; install -D -m 0755 vely.sel -t $(DESTDIR)$(V_LIB)/selinux ; if [ "$(VV_NO_SEL)" != "1" ]; then ./vely.sel "$(DESTDIR)$(V_LIB)/selinux" "$(DESTDIR)$(V_VV_DATADIR)" "$(DESTDIR)$(V_BIN)"; fi ; fi

.PHONY: uninstall
uninstall:
	@if [ ! -f "$(DESTDIR)$(V_LIB)/sys" ]; then echo "Vely not installed, cannot uninstall."; exit -1; else echo "Uninstalling Vely..."; fi
	. $(DESTDIR)$(V_LIB)/sys
	rm -rf $(DESTDIR)$(V_INC)
	rm -f $(DESTDIR)$(V_BIN)/vv
	rm -f $(DESTDIR)$(V_BIN)/vf
	rm -f $(DESTDIR)$(V_MAN)/*.2vv
	rm -rf $(DESTDIR)$(V_VV_DOCS)
	rm -rf $(DESTDIR)$(V_LIB)

.PHONY: binary
binary:build
	@;

.PHONY: build
build: libfcgively.so libvelyfcli.so libvelyfsrv.so libvely.so libvelydb.so libvelysec.so libvelymys.so libvelylite.so libvelypg.so libvelycurl.so v1.o stub_sqlite.o stub_postgres.o stub_mariadb.o stub_gendb.o stub_curl.o stub_fcgi.o stub_crypto.o stub_after.o stub_before.o stub_startup.o vf 
	@echo "Building version $(BUILDVER).$(BUILDREL)"
	$(CC) -o v1 v1.o chandle.o velyrtc.o velymem.o $(LDFLAGS) 

.PHONY: clean
clean:
	touch *.c
	touch *.h
	rm -rf debian/vely
	rm -rf *.tar.gz



#
# Other than VELY preprocessor, we do NOT use any libraries at customer's site - 
# the Makefile for application (such as in hello world example) will link with
# those libraries AT customer site.
#
v1.o: v1.c
	$(CC) -c -o $@ $< $(CFLAGS) 

vf: vf.o 
	$(CC) -o vf vf.o $(LDFLAGS)

libfcgively.so: chandle.o json.o hash.o utf8.o fcgi_velyrt.o velyrtc.o velymem.o 
	rm -f libfcgively.so
	$(CC) -shared -o libfcgively.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libfcgively.so ; fi

libvely.so: chandle.o json.o hash.o utf8.o velyrt.o velyrtc.o velymem.o 
	rm -f libvely.so
	$(CC) -shared -o libvely.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvely.so ; fi

libvelypg.so: pg.o 
	rm -f libvelypg.so
	$(CC) -shared -o libvelypg.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelypg.so ; fi

libvelylite.so: lite.o 
	rm -f libvelylite.so
	$(CC) -shared -o libvelylite.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelylite.so ; fi

libvelymys.so: mys.o 
	rm -f libvelymys.so
	$(CC) -shared -o libvelymys.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelymys.so ; fi

libvelydb.so: db.o 
	rm -f libvelydb.so
	$(CC) -shared -o libvelydb.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelydb.so ; fi

libvelysec.so: sec.o 
	rm -f libvelysec.so
	$(CC) -shared -o libvelysec.so  $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelysec.so ; fi

libvelycurl.so: curl.o 
	rm -f libvelycurl.so
	$(CC) -shared -o libvelycurl.so $^ 
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelycurl.so ; fi

utf8.o: utf8.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

json.o: json.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

hash.o: hash.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

chandle.o: chandle.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

pg.o: pg.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

db.o: db.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

lite.o: lite.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

mys.o: mys.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

sec.o: sec.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

stub_after.o: stub_after.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

stub_before.o: stub_before.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

stub_startup.o: stub_startup.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

stub_crypto.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_CRYPTO $< $(CFLAGS) 

stub_curl.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_CURL $< $(CFLAGS) 

stub_fcgi.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_FCGI $< $(CFLAGS) 

stub_sqlite.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_STUB_SQLITE $< $(CFLAGS) 

stub_postgres.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_STUB_POSTGRES $< $(CFLAGS) 

stub_mariadb.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_STUB_MARIADB $< $(CFLAGS) 

stub_gendb.o: stub.c vely.h
	$(CC) -c -o $@ -DVV_STUB_GENDB $< $(CFLAGS) 

curl.o: curl.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

velyrtc.o: velyrtc.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)

fcgi_velyrt.o: velyrt.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS)  

velyrt.o: velyrt.c vely.h
	$(CC) -c -DVV_COMMAND -o $@ $< $(CFLAGS)  

velymem.o: velymem.c vely.h
	$(CC) -c -o $@ $< $(CFLAGS) 

vf.o: vf.c 
	$(CC) -c -o $@ $< $(CFLAGS) 

libvelyfcli.so: fcli.c vfcgi.h
	rm -f libvelyfcli.so
	$(CC) -shared -o libvelyfcli.so $^ $(CFLAGS)
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelyfcli.so ; fi

libvelyfsrv.so: fcli.c vfcgi.h
	rm -f libvelyfsrv.so
	$(CC) -shared -o libvelyfsrv.so $^ $(CFLAGS) -DVV_VELYSRV
	if [ "$(DEBUGINFO)" != "1" ]; then strip --strip-unneeded libvelyfsrv.so ; fi


