AM_CPPFLAGS = -I$(srcdir)/..

TESTS = RAPIDJSON_TEST

check_PROGRAMS = $(TESTS)

RAPIDJSON_TEST_SOURCES = test_rapidjson.cpp test_main.cpp

test_main.cpp: $(top_srcdir)/unittest/ir_test_main.cpp
	$(LN_S) $< $@

RAPIDJSON_TEST_LDADD = -lcppunit

CLEANFILES = test_main.cpp
