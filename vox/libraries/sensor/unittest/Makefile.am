AM_CPPFLAGS = -I$(srcdir)/.. \
	-I$(srcdir)/../../log    \
	-I$(srcdir)/../../motor  \
	-I$(srcdir)/../../socket \
	-I$(srcdir)/../../util   \
	-I$(srcdir)/../../wiring \
	-DSRCDIR=\"$(srcdir)\"   \
	$(NULL)

TESTS = SENSOR_TEST

check_PROGRAMS = $(TESTS)

SENSOR_TEST_SOURCES = test_sensor.cpp test_main.cpp

SENSOR_TEST_LDADD = ../libsensor.la \
	$(top_builddir)/libvox.la -lrt -lwiringPi -lcppunit

test_main.cpp: $(top_srcdir)/unittest/test_main.cpp
	$(LN_S) $< $@

CLEANFILES = test_main.cpp

#clean-local:
#	rm -rf test
