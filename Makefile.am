DIST_SUBDIRS = m4 t

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libcrono.la

include_HEADERS = crono.h

bin_PROGRAMS = crono

libcrono_la_SOURCES = \
	crono.c crono.h

libcrono_la_LDFLAGS = -avoid-version

crono_SOURCES = main.c utils.c
crono_LDADD = libcrono.la

test: all
	cd t && $(MAKE) test

