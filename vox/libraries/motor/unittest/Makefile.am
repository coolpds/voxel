AM_CPPFLAGS = -I$(srcdir)/.. \
	-I$(srcdir)/../../log    \
	-I$(srcdir)/../../motor  \
	-I$(srcdir)/../../socket \
	-I$(srcdir)/../../thread \
	-I$(srcdir)/../../util   \
	-I$(srcdir)/../../wiring \
	-I$(srcdir)/../../wiring/src \
	-DSRCDIR=\"$(srcdir)\"   \
	$(NULL)

TESTS = MOTOR_TEST

check_PROGRAMS = $(TESTS)

MOTOR_TEST_SOURCES = test_motor.cpp test_main.cpp

MOTOR_TEST_LDADD = ../libmotor.la \
	$(top_builddir)/libvox.la -lrt -lwiringPi -lm -lcppunit

test_main.cpp: $(top_srcdir)/unittest/test_main.cpp
	$(LN_S) $< $@

CLEANFILES = test_main.cpp

#clean-local:
#	rm -rf test
