AM_CPPFLAGS = -I$(srcdir)/../../libraries/exception  \
	-I$(srcdir)/../../libraries/log       \
	-I$(srcdir)/../../libraries/motor     \
	-I$(srcdir)/../../libraries/rapidjson \
	-I$(srcdir)/../../libraries/sensor    \
	-I$(srcdir)/../../libraries/socket    \
	-I$(srcdir)/../../libraries/thread    \
	-I$(srcdir)/../../libraries/util      \
	-I$(srcdir)/../../libraries/wiring

bin_PROGRAMS = vox

vox_SOURCES = \
	vox_control.cpp vox_control.hpp \
	vox_main.cpp vox_main.hpp \
	vox_misc.cpp vox_misc.hpp \
	vox_player.cpp vox_player.hpp \
	vox_sensor.cpp vox_sensor.hpp \
	vox_vision.cpp vox_vision.hpp \
	vox_voice.cpp vox_voice.hpp

vox_LDFLAGS = -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l'

vox_LDADD = ../../libvox.la \
	-lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui \
	-L/usr/lib/uv4l/uv4lext/armv6l -luv4lext \
	-lmpg123 -lao -lcurl \
	-lpthread -lwiringPi -lm -ldl -lrt $(LIBZ_LIBS)
