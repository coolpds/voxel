###############################################################
#
# Purpose: Makefile for "M-JPEG Streamer"
# Author.: Tom Stoeveken (TST)
# Version: 0.3
# License: GPL
#
###############################################################

CC = gcc

OTHER_HEADERS = ../../mjpg_streamer.h ../../utils.h ../output.h ../input.h

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -Wuninitialized

#CFLAGS += -g -DDEBUG # uncomment it to get debug symbols compiled in and debug messages shown
#CFLAGS += -DMANAGMENT # experimental feature to control the clients maximum allowed thread count

LFLAGS += -lpthread -ldl -lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_objdetect
#-L/usr/lib/uv4l/uv4lext/armv6l -luv4lext

ifeq ($(WXP_COMPAT),true)
CFLAGS += -DWXP_COMPAT
endif

all: output_http.so

clean:
    rm -f *.a *.o core *~ *.so *.lo

output_http.so: $(OTHER_HEADERS) output_http.c httpd.lo
    $(CC) $(CFLAGS) $(LFLAGS) -o $@ output_http.c httpd.lo

httpd.lo: $(OTHER_HEADERS) httpd.h httpd.c
    $(CC) -c $(CFLAGS) -o $@ httpd.c
