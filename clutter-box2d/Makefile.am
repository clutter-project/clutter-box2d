INCLUDES = -I$(top_srcdir)/box2d -I$(top_srcdir)/box2d/Box2D -I$(srcdir) -I$(top_srcdir)

AM_CFLAGS = @MAINTAINER_CFLAGS@

AM_CPPFLAGS = @DEPS_CFLAGS@

lib_LTLIBRARIES = libclutter-box2d-@CLUTTER_BOX2D_API_VERSION@.la

MARSHALFILES = clutter-box2d-marshal.c clutter-box2d-marshal.h

BUILT_SOURCES = $(MARSHALFILES)

libclutter_box2d_@CLUTTER_BOX2D_API_VERSION@_la_LIBADD  = @DEPS_LIBS@ $(top_builddir)/box2d/libbox2d.la
libclutter_box2d_@CLUTTER_BOX2D_API_VERSION@_la_LDFLAGS =	\
	@CLUTTER_BOX2D_LT_LDFLAGS@				\
	-export-symbols-regex "^clutter.*"			\
	$(NULL)
libclutter_box2d_@CLUTTER_BOX2D_API_VERSION@_la_SOURCES = \
    clutter-box2d.cpp           \
    clutter-box2d-child.cpp     \
    clutter-box2d-joint.cpp     \
    clutter-box2d-child.h       \
    clutter-box2d-util.c        \
    clutter-box2d-collision.cpp \
    clutter-box2d-contact.cpp   \
    clutter-box2d-contact.h     \
    clutter-box2d-private.h     \
    $(BUILT_SOURCES)

clutter_box2dheaders_HEADERS =  \
    clutter-box2d.h             \
    clutter-box2d-child.h       \
    clutter-box2d-joint.h       \
    clutter-box2d-util.h        \
    clutter-box2d-collision.h   \
    clutter-box2d-marshal.h

clutter_box2dheadersdir = $(includedir)/clutter-1.0/clutter-box2d

DISTCLEANFILES = $(MARSHALFILES)
CLEANFILES=
EXTRA_DIST = 

glib_marshal_list = clutter-box2d-marshal.list
glib_marshal_prefix = _clutter_box2d_marshal
include $(top_srcdir)/build/autotools/Makefile.am.marshal

INTROSPECTION_GIRS =

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION

introspection_files = \
	$(top_srcdir)/clutter-box2d/clutter-box2d.h \
	$(top_srcdir)/clutter-box2d/clutter-box2d-joint.h \
	$(top_srcdir)/clutter-box2d/clutter-box2d-util.h

ClutterBox2D-0.10.gir: $(INTROSPECTION_SCANNER) Makefile libclutter-box2d-@CLUTTER_BOX2D_API_VERSION@.la

ClutterBox2D_0_10_gir_SCANNERFLAGS = --strip-prefix=ClutterBox2D --add-include-path=$(top_builddir)
ClutterBox2D_0_10_gir_INCLUDES     = Clutter-1.0
ClutterBox2D_0_10_gir_PACKAGES     = clutter-1.0
ClutterBox2D_0_10_gir_CFLAGS       = $(INCLUDES) $(AM_CPPFLAGS)
ClutterBox2D_0_10_gir_LIBS         = libclutter-box2d-@CLUTTER_BOX2D_API_VERSION@.la
ClutterBox2D_0_10_gir_FILES        = $(introspection_files)

INTROSPECTION_GIRS += ClutterBox2D-0.10.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelibs_DATA)
endif
