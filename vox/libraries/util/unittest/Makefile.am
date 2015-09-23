AM_CPPFLAGS = -I$(srcdir)/.. \
	-I$(srcdir)/../../log    \
	-I$(srcdir)/../../socket \
	-I$(srcdir)/../../util   \
	-DSRCDIR=\"$(srcdir)\"   \
	$(NULL)

TESTS = UTIL_TEST

check_PROGRAMS = $(TESTS)

UTIL_TEST_SOURCES = test_util.cpp test_main.cpp

UTIL_TEST_LDADD = ../libutil.la \
	$(top_builddir)/libvox.la -lcppunit

test_main.cpp: $(top_srcdir)/unittest/test_main.cpp
	$(LN_S) $< $@

CLEANFILES = test_main.cpp

#clean-local:
#	rm -rf test
