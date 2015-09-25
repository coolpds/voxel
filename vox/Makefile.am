SUBDIRS = libraries . executables

lib_LTLIBRARIES = libvox.la

libvox_la_SOURCES =

nodist_EXTRA_libvox_la_SOURCES = dummy.cpp

libvox_la_LIBADD = libraries/exception/libexception.la \
	libraries/log/liblog.la \
	libraries/motor/libmotor.la \
	libraries/sensor/libsensor.la \
	libraries/socket/libsocket.la \
	libraries/thread/libthread.la \
	libraries/util/libutil.la

libvox_la_LDFLAGS = -version-info 0:0:0
