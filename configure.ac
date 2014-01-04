AC_PREREQ([2.69])
AC_INIT([ulcd43], [1.0], [kimtjen@gmail.com])
LT_INIT
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PATH_CHECK

AC_CONFIG_SRCDIR([src/test.c])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for the RPI serial bug.
AC_ARG_WITH([serial-bug],
    AS_HELP_STRING([--with-serial-bug], [assume that new serial connections send garbage at open()])
)
AS_IF([test "x$serial_bug" != "xno"], [
    AC_DEFINE(HAVE_SERIAL_BUG, 1, [Assume that open() sends garbage through the serial connection])
])

# Checks for the RPI serial bug.
AC_ARG_ENABLE([serial-debug],
    AS_HELP_STRING([--enable-serial-debug], [Print serial communications to stderr])
)
AS_IF([test "x$serial_debug" == "xyes"], [
    AC_DEFINE(SERIAL_DEBUG, 1, [Print serial communications to stderr])
])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
])
AC_OUTPUT
