AC_PREREQ(2.69)
AC_INIT([alphajunoctl],[1],[dlbeswick@gmail.com])
AC_CONFIG_SRCDIR([src/alphajunoctl.c])
AM_INIT_AUTOMAKE([-Wall -Werror 1.16.1 foreign subdir-objects])
AM_PROG_AR
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile])

REQ_GLIB=2.44.0
REQ_GST=1.2.0

GST_MAJORMINOR=1.0
AC_DEFINE_UNQUOTED(GST_MAJORMINOR, "$GST_MAJORMINOR", [gstreamer series])
AC_SUBST(GST_MAJORMINOR)

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(BASE_DEPS, \
  glib-2.0 >= $REQ_GLIB gobject-2.0  >= $REQ_GLIB gthread-2.0 >= $REQ_GLIB \
  gmodule-2.0 >= $REQ_GLIB gio-2.0 >= $REQ_GLIB \
  gstreamer-1.0 >= $REQ_GST gstreamer-controller-1.0 >= $REQ_GST \
  gstreamer-base-1.0 >= $REQ_GST \
  gstreamer-plugins-base-1.0 >= $REQ_GST \
  gstreamer-audio-1.0 >= $REQ_GST \
  gstreamer-fft-1.0 >= $REQ_GST \
)

PKG_CHECK_VAR([GLIB_MKENUMS], [glib-2.0], [glib_mkenums])

# We don't want *.a files
LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# Ensure buzztrax library files will be found when installing to a custom buzztrax prefix.
LDFLAGS=-L$prefix/lib

# Checks for libraries.
AC_CHECK_LIB([buzztrax-gst], [gstbt_audio_synth_get_type], [], [AC_MSG_ERROR([buzztrax shared lib not found])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h malloc.h stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset pow sqrt strchr])

dnl enable runtime debugging code (this is more like a developer build mode)
AC_MSG_CHECKING(whether to enable runtime debugging code)
AC_ARG_ENABLE(
	debug,
	AS_HELP_STRING([--enable-debug],[enable runtime debugging code (default=no)]),
	,
	[enable_debug="no"])
AC_MSG_RESULT($enable_debug)
if test "$enable_debug" = "yes"; then
	AC_DEFINE(USE_DEBUG, [1], [enable runtime debugging code])
	DEBUG_CFLAGS="-O0 -Wall -Werror -g"
else
	DEBUG_CFLAGS=""
fi

plugindir="\$(libdir)/gstreamer-$GST_MAJORMINOR"
AC_SUBST(plugindir)
presetdir="\$(datadir)/Gear"
AC_SUBST(presetdir)
userdocdir="\$(datadir)/Gear"
AC_SUBST(userdocdir)
demosongdir="\$(datadir)/buzztrax/songs/Gear"
AC_SUBST(demosongdir)

AC_OUTPUT

echo "
Configuration
	Version                    : ${VERSION}
	Source code location       : ${srcdir}
	Prefix                     : ${prefix}
	Compiler                   : ${CC}

	Debug                      : ${enable_debug}
"
