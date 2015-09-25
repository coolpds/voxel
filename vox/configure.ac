#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([vox], [0.0.0], [bug@localhost])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AS_IF([test "x$GXX" = xyes], [CXX="$CXX -std=c++0x"])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MKTIME
AC_CHECK_FUNCS([localtime_r])

# workaround for libtool not linking against pthread when linking using c++
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SYS_LARGEFILE
AC_SEARCH_LIBS([clock_gettime], [rt])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libraries/Makefile])
AC_CONFIG_FILES([libraries/exception/Makefile])
AC_CONFIG_FILES([libraries/log/Makefile])
AC_CONFIG_FILES([libraries/motor/Makefile])
AC_CONFIG_FILES([libraries/motor/unittest/Makefile])
AC_CONFIG_FILES([libraries/rapidjson/Makefile])
AC_CONFIG_FILES([libraries/rapidjson/unittest/Makefile])
AC_CONFIG_FILES([libraries/sensor/Makefile])
AC_CONFIG_FILES([libraries/sensor/unittest/Makefile])
AC_CONFIG_FILES([libraries/socket/Makefile])
AC_CONFIG_FILES([libraries/thread/Makefile])
AC_CONFIG_FILES([libraries/util/Makefile])
AC_CONFIG_FILES([libraries/util/unittest/Makefile])
AC_CONFIG_FILES([executables/Makefile])
AC_CONFIG_FILES([executables/vox/Makefile])

AC_OUTPUT

