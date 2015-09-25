SUBDIRS = . unittest

AM_CPPFLAGS = -I$(srcdir)/../log \
	-I$(srcdir)/../thread \
	-I$(srcdir)/../util \
	-I$(srcdir)/../wiring \
	-I$(srcdir)/../wiring/src

noinst_LTLIBRARIES = libmotor.la

libmotor_la_SOURCES = step.hpp step.cpp \
	servo.hpp servo.cpp \
	pwm.hpp pwm.cpp
