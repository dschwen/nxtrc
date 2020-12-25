#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#

AC_PREREQ(2.63)
AC_INIT([nxtrc], [2.3], [gianpiero.puccioni@isc.cnr.it],
        [nxtrc], [http://www.scienzaludica.it/index.php?page=88])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([nxtrc_func.h])
#AC_CONFIG_AUX_DIR(.auto)


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


# Checks for libraries.
PKG_CHECK_MODULES(BLUEZ, bluez)
AC_SUBST(BLUEZ_CFLAGS)
AC_SUBST(BLUEZ_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
