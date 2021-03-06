dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])

m4_define(libmatemixer_major_version, 1)
m4_define(libmatemixer_minor_version, 10)
m4_define(libmatemixer_micro_version, 0)
m4_define(libmatemixer_interface_age, 0)
m4_define(libmatemixer_version,
         [libmatemixer_major_version.libmatemixer_minor_version.libmatemixer_micro_version])

AC_INIT([libmatemixer],
        [libmatemixer_version],
        [http://www.mate-desktop.org])

AC_DEFINE(LIBMATEMIXER_MAJOR_VERSION, libmatemixer_major_version, [Libmatemixer major version])
AC_DEFINE(LIBMATEMIXER_MINOR_VERSION, libmatemixer_minor_version, [Libmatemixer minor version])
AC_DEFINE(LIBMATEMIXER_MICRO_VERSION, libmatemixer_micro_version, [Libmatemixer micro version])

AC_SUBST(LIBMATEMIXER_MAJOR_VERSION, libmatemixer_major_version)
AC_SUBST(LIBMATEMIXER_MINOR_VERSION, libmatemixer_minor_version)
AC_SUBST(LIBMATEMIXER_MICRO_VERSION, libmatemixer_micro_version)
AC_SUBST(LIBMATEMIXER_VERSION,       libmatemixer_version)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([libmatemixer/matemixer.c])

AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz foreign tar-ustar check-news])

# Use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],
         [AM_SILENT_RULES([yes])],
         [AM_DEFAULT_VERBOSITY=1 AC_SUBST(AM_DEFAULT_VERBOSITY)])

# Checks for required programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h math.h sys/types.h unistd.h libintl.h locale.h])

# =======================================================================
# Libtool
# =======================================================================
LT_CURRENT=0
LT_REVISION=0
LT_AGE=0
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

LT_PREREQ([2.2])
LT_INIT

# =======================================================================
# Gettext
# =======================================================================
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=libmatemixer
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package])

AM_GLIB_GNU_GETTEXT

# =======================================================================
# Check for required packages
# =======================================================================
PKG_PROG_PKG_CONFIG

GLIB_REQUIRED_VERSION=2.36.0

PKG_CHECK_MODULES(GLIB, [
        glib-2.0 >= $GLIB_REQUIRED_VERSION
        gobject-2.0 >= $GLIB_REQUIRED_VERSION
        gmodule-2.0 >= $GLIB_REQUIRED_VERSION
])

GTK_DOC_CHECK([1.10], [--flavour no-tmpl])

# =======================================================================
# Check for backend module support
# =======================================================================
# -----------------------------------------------------------------------
# Null
# -----------------------------------------------------------------------
AC_ARG_ENABLE([null],
              AS_HELP_STRING([--enable-null],
                             [Enable Null backend module @<:@default=yes@:>@]),
              enable_null=$enableval, enable_null=yes)

if test "x$enable_null" != "xno" ; then
  AC_DEFINE(HAVE_NULL, [], [Define if we have Null module])
  have_null=yes
else
  have_null=no
fi

AM_CONDITIONAL(HAVE_NULL, test "x$have_null" = "xyes")
AC_SUBST(HAVE_NULL)

# -----------------------------------------------------------------------
# PulseAudio
# -----------------------------------------------------------------------
PA_REQUIRED_VERSION=0.9.23

AC_ARG_ENABLE([pulseaudio],
              AS_HELP_STRING([--enable-pulseaudio],
                             [Enable PulseAudio backend module @<:@default=auto@:>@]),
              enable_pulseaudio=$enableval, enable_pulseaudio=auto)

if test "x$enable_pulseaudio" != "xno"; then
  PKG_CHECK_MODULES(PULSEAUDIO,
          libpulse >= $PA_REQUIRED_VERSION
          libpulse-mainloop-glib >= $PA_REQUIRED_VERSION,
          have_pulseaudio=yes,
          have_pulseaudio=no)

  if test "x$enable_pulseaudio" = "xyes" -a "x$have_pulseaudio" = "xno"; then
    AC_MSG_ERROR([PulseAudio support explicitly requested but dependencies not found])
  fi

  if test "x$have_pulseaudio" = "xyes" ; then
    AC_DEFINE(HAVE_PULSEAUDIO, [], [Define if we have PulseAudio support])
  fi
else
  have_pulseaudio=no
fi

AM_CONDITIONAL(HAVE_PULSEAUDIO, test "x$have_pulseaudio" = "xyes")

AC_SUBST(HAVE_PULSEAUDIO)
AC_SUBST(PULSEAUDIO_CFLAGS)
AC_SUBST(PULSEAUDIO_LIBS)

# -----------------------------------------------------------------------
# ALSA
# -----------------------------------------------------------------------
# This is set accordingly to the version reported by liboss4-salsa
ALSA_REQUIRED_VERSION=1.0.5

AC_ARG_ENABLE([alsa],
              AS_HELP_STRING([--enable-alsa],
                             [Enable ALSA backend module @<:@default=auto@:>@]),
              enable_alsa=$enableval, enable_alsa=auto)

if test "x$enable_alsa" != "xno"; then
  PKG_CHECK_MODULES(ALSA, [
        alsa >= $ALSA_REQUIRED_VERSION
        gthread-2.0 >= $GLIB_REQUIRED_VERSION
        ],
        have_alsa=yes,
        have_alsa=no)

  if test "x$enable_alsa" = "xyes" -a "x$have_alsa" = "xno"; then
    AC_MSG_ERROR([ALSA support explicitly requested but dependencies not found])
  fi

  if test "x$have_alsa" = "xyes" ; then
    AC_DEFINE(HAVE_ALSA, [], [Define if we have ALSA support])
  fi
else
  have_alsa=no
fi

AM_CONDITIONAL(HAVE_ALSA, test "x$have_alsa" = "xyes")

AC_SUBST(HAVE_ALSA)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

# -----------------------------------------------------------------------
# OSS
# -----------------------------------------------------------------------
AC_ARG_ENABLE([oss],
              AS_HELP_STRING([--enable-oss],
                             [Enable OSS backend module @<:@default=no@:>@]),
              enable_oss=$enableval, enable_oss=no)

if test "x$enable_oss" != "xno"; then
  AC_CHECK_HEADERS([soundcard.h sys/soundcard.h machine/soundcard.h])
  if test "x$ac_cv_header_soundcard_h" = "xyes" -o \
          "x$ac_cv_header_sys_soundcard_h" = "xyes" -o \
          "x$ac_cv_header_machine_soundcard_h" = "xyes"; then
    AC_CHECK_LIB([ossaudio], [_oss_ioctl], [OSS_LIBS="-lossaudio"])
    have_oss=yes
  else
    have_oss=no
  fi

  if test "x$enable_oss" = "xyes" -a "x$have_oss" = "xno"; then
    AC_MSG_ERROR([OSS support explicitly requested but dependencies not found])
  fi

  if test "x$have_oss" = "xyes" ; then
    AC_DEFINE(HAVE_OSS, [], [Define if we have OSS support])
  fi
else
  have_oss=no
fi

AM_CONDITIONAL(HAVE_OSS, test "x$have_oss" = "xyes")

AC_SUBST(HAVE_OSS)
AC_SUBST(OSS_CFLAGS)
AC_SUBST(OSS_LIBS)

# =======================================================================
# Compiler warnings
# =======================================================================
MATE_COMPILE_WARNINGS([maximum])
MATE_CXX_WARNINGS

# Turn on the additional warnings last, so warnings don't affect other tests.
AC_ARG_ENABLE(more-warnings,
        [AC_HELP_STRING([--enable-more-warnings],
        [Maximum compiler warnings])],
        set_more_warnings="$enableval",[
                if test -d $srcdir/.git; then
                        set_more_warnings=yes
                else
                        set_more_warnings=no
                fi
        ])

AC_MSG_CHECKING(for more warnings)
if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
        AC_MSG_RESULT(yes)
        CFLAGS="-Wall -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wcast-align -Wsign-compare $CFLAGS"

        for option in -Wno-unused-parameter -Wno-strict-aliasing -Wno-sign-compare; do
                SAVE_CFLAGS="$CFLAGS"
                CFLAGS="$CFLAGS $option"
                AC_MSG_CHECKING([whether gcc understands $option])
                AC_TRY_COMPILE([], [],
                        has_option=yes,
                        has_option=no,)
                if test $has_option = no; then
                        CFLAGS="$SAVE_CFLAGS"
                fi
                AC_MSG_RESULT($has_option)
                unset has_option
                unset SAVE_CFLAGS
        done
        unset option
else
        AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)

# =======================================================================
# Finish
# =======================================================================
AC_CONFIG_FILES([
Makefile
libmatemixer/Makefile
backends/Makefile
backends/null/Makefile
backends/pulse/Makefile
backends/alsa/Makefile
backends/oss/Makefile
data/Makefile
data/libmatemixer.pc
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
examples/Makefile
po/Makefile.in
])

AC_OUTPUT

echo "

                    ${PACKAGE} ${VERSION}
                    ============


        Prefix:                      ${prefix}
        Source code location:        ${srcdir}
        Compiler:                    ${CC}
        CFLAGS:                      ${CFLAGS}

        Build Null module:           $have_null
        Build PulseAudio module:     $have_pulseaudio
        Build ALSA module:           $have_alsa
        Build OSS module:            $have_oss

"
