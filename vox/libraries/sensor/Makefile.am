SUBDIRS = . unittest

AM_CPPFLAGS = -I$(srcdir)/../log \
	-I$(srcdir)/../util \
	-I$(srcdir)/../wiring

noinst_LTLIBRARIES = libsensor.la

libsensor_la_SOURCES = dht.hpp dht.cpp \
	usonic.hpp usonic.cpp
