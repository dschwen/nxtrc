# Makefile.am
# Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>

bin_PROGRAMS = nxtrc

EXTRA_DIST = Readme.txt SendDat.nxc GetData.c COPYING

dist_man1_MANS = nxtrc.1 

nxtrc_SOURCES = nxtrc.c nxtrc_func.c nxtrc_func.h

nxtrc_LDADD = $(BLUEZ_LIBS)
nxtrc_CFLAGS = $(BLUEZ_CFLAGS)

dist_noinst_SCRIPTS = autogen.sh

MAINTAINERCLEANFILES = Makefile.in depcomp missing install-sh configure aclocal.m4
