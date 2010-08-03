# ClutterBox2D package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress development, (eg. from git)
# An even micro number indicates a released version.
m4_define(clutter_box2d_version_major, 0)
m4_define(clutter_box2d_version_minor, 10)
m4_define(clutter_box2d_version_micro, 1)

m4_define([clutter_box2d_version],
          [clutter_box2d_version_major.clutter_box2d_version_minor.clutter_box2d_version_micro])

m4_define([clutter_box2d_api_version],
          [clutter_box2d_version_major.clutter_box2d_version_minor])

# increase the interface age by 2 for each release; if the API changes,
# set to 0. interface_age and binary_age are used to create the soname
# of the shared object:
#
#  (<minor> * 100 + <micro>) - <interface_age>
#
# this allows using the same soname for different micro-releases in case
# no API was added or deprecated. for instance:
#
#   1.2.0  -> 100 * 2 + 0  = 200, interface age = 0 -> 200
#   1.2.2  -> 100 * 2 + 2  = 202, interface age = 2 -> 200
#   1.2.4  -> 100 * 2 + 4  = 204, interface age = 4 -> 200
#   [ API addition, deprecation ]
#   1.2.6  -> 100 * 2 + 6  = 206, interface age = 0 -> 206
#   1.2.8  -> 100 * 2 + 8  = 208, interface age = 2 -> 206
#   1.2.10 -> 100 * 2 + 10 = 210, interface age = 4 -> 206
#   ...

m4_define([clutter_box2d_interface_age], [0])
m4_define([clutter_box2d_binary_age],
          [m4_eval(100 * clutter_box2d_version_minor + clutter_box2d_version_micro)])

# Autoconf configuration
AC_PREREQ(2.53)

AC_INIT([clutter-box2d],
        [clutter_box2d_version],
        [http://bugzilla.clutter-project.org/enter_bug.cgi?product=clutter-box2d])

AC_CONFIG_SRCDIR([clutter-box2d/clutter-box2d.h])
AC_CONFIG_MACRO_DIR([build/autotools])

# Automake configuration
AM_INIT_AUTOMAKE([1.9])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# libtool configuration
LT_PREREQ([2.2.6])
LT_INIT([win32-dll disable-static])

# export a few variables
CLUTTER_BOX2D_MAJOR_VERSION=clutter_box2d_major
CLUTTER_BOX2D_MINOR_VERSION=clutter_box2d_minor
CLUTTER_BOX2D_MICRO_VERSION=clutter_box2d_micro
CLUTTER_BOX2D_VERSION=clutter_box2d_version
CLUTTER_BOX2D_API_VERSION=clutter_box2d_api_version
AC_SUBST(CLUTTER_BOX2D_MAJOR_VERSION)
AC_SUBST(CLUTTER_BOX2D_MINOR_VERSION)
AC_SUBST(CLUTTER_BOX2D_MICRO_VERSION)
AC_SUBST(CLUTTER_BOX2D_API_VERSION)

m4_define([lt_current],
          [m4_eval(clutter_box2d_binary_age - clutter_box2d_interface_age)])
m4_define([lt_revision], [clutter_box2d_interface_age])
m4_define([lt_age], [m4_eval(clutter_box2d_binary_age - clutter_box2d_interface_age)])
CLUTTER_BOX2D_LT_CURRENT=lt_current
CLUTTER_BOX2D_LT_REV=lt_revision
CLUTTER_BOX2D_LT_AGE=lt_age
CLUTTER_BOX2D_LT_VERSION="$CLUTTER_BOX2D_LT_CURRENT:$CLUTTER_BOX2D_LT_REV:$CLUTTER_BOX2D_LT_AGE"
CLUTTER_BOX2D_LT_LDFLAGS="-version-info $CLUTTER_BOX2D_LT_VERSION"

AC_SUBST(CLUTTER_BOX2D_LT_VERSION)
AC_SUBST(CLUTTER_BOX2D_LT_LDFLAGS)

dnl ========================================================================

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strcasecmp strdup])

dnl ========================================================================

pkg_modules="clutter-1.0 >= 1.0.0"
PKG_CHECK_MODULES(DEPS, [$pkg_modules])

AS_COMPILER_FLAGS([MAINTAINER_CFLAGS], ["-Wall"])

AC_SUBST(MAINTAINER_CFLAGS)

dnl = GTK Doc check ========================================================

GTK_DOC_CHECK([1.13])

dnl ========================================================================

GOBJECT_INTROSPECTION_CHECK([0.9.0])

AC_CONFIG_FILES([
    Makefile
    build/Makefile
    build/autotools/Makefile
    clutter-box2d.pc
    examples/Makefile
    box2d/Makefile
    clutter-box2d/Makefile
    doc/Makefile
    doc/reference/Makefile
    doc/reference/version.xml
])

AC_OUTPUT

echo ""
echo "           ClutterBox2D $VERSION"
echo "          ====================="
echo ""
echo "   Documentation: ${enable_gtk_doc}"
echo ""
