# Init Autoconf
AC_PREREQ([2.68])
AC_INIT([gst-gz], [1.0], [nnoell3@gmail.com])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_MACRO_DIRS([m4])

# Init Automake
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_CONFIG_HEADER(config.h)
AM_SILENT_RULES([yes])

# Init M4
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Init Libtool
LT_PREREQ([2.2])
LT_INIT([dlopen shared])

# Enable Args
AC_ARG_ENABLE(gstreamer-0.10,
  AS_HELP_STRING([--enable-gstreamer-0.10], [Uses gstreamer 0.10 API instead of 1.0 (default=no)]),
  [
    case "${enableval}" in
      yes) ENABLE_GSTREAMER_010=yes ;;
      no)  ENABLE_GSTREAMER_010=no ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --enable-gstreamer-0.10) ;;
    esac
  ],
[ENABLE_GSTREAMER_010=no])
AM_CONDITIONAL(ENABLE_GSTREAMER_010, test "x$ENABLE_GSTREAMER_010" = "xyes")

if test x$ENABLE_GSTREAMER_010  = xyes; then
  GST_PLUGIN_CFLAGS="-DUSE_GSTREAMER_010 -I/usr/include/gstreamer-0.10 -I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include"
  GST_PLUGIN_LDFLAGS="-module -avoid-version -pthread -lgstbase-0.10 -lgstreamer-0.10 -lgobject-2.0 -lglib-2.0 -lz -lbz2"
  plugindir="/usr/lib/gstreamer-0.10"
else
  GST_PLUGIN_CFLAGS="-I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include"
  GST_PLUGIN_LDFLAGS="-module -avoid-version -pthread -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lz -lbz2"
  plugindir="/usr/lib/gstreamer-1.0"
fi
AC_SUBST([GST_PLUGIN_CFLAGS])
AC_SUBST([GST_PLUGIN_LDFLAGS])
AC_SUBST(plugindir)

# Check Programs
AC_PROG_CC

# Check Libraries
AC_CHECK_LIB([z], [inflate], [], [AC_MSG_ERROR(Cannot find -lz)])
AC_CHECK_LIB([bz2], [BZ2_bzDecompress], [], [AC_MSG_ERROR(Cannot find -lbz2)])
if test x$USE_GSTREAMER_010  = xyes; then
	AC_CHECK_LIB([gstreamer-0.10], [gst_init], [], [AC_MSG_ERROR(Cannot find -lgstreamer-0.10)])
	AC_CHECK_LIB([gstbase-0.10], [gst_base_transform_is_passthrough], [], [AC_MSG_ERROR(Cannot find -lgstbase-0.10)])
else
	AC_CHECK_LIB([gstreamer-1.0], [gst_init], [], [AC_MSG_ERROR(Cannot find -lgstreamer-1.0)])
	AC_CHECK_LIB([gstbase-1.0], [gst_base_transform_is_passthrough], [], [AC_MSG_ERROR(Cannot find -lgstbase-1.0)])
fi

# Check Headers
AC_CHECK_HEADERS([zlib.h],[],[AC_MSG_ERROR(Cannot find zlib.h)],[])
AC_CHECK_HEADERS([bzlib.h],[],[AC_MSG_ERROR(Cannot find bzlib.h)],[])

AC_OUTPUT

echo "
  Configuration
    Version                : ${VERSION}
    Compiler               : ${CC}
    Use Gstreamer 0.10 API : ${ENABLE_GSTREAMER_010}
    Install location       : ${plugindir}
"
