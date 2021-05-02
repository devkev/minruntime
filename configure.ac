AC_PREREQ(2.5)
AC_INIT(minruntime, [0.1], [kev@pulo.com.au])
AC_CONFIG_SRCDIR([src/libminruntime.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Check for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED

# Choose shared library using libtool.
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

# Check for standard headers, etc.
AC_HEADER_STDC
AC_C_CONST

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
