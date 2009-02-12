INCLUDES = @GCC_FLAGS@ @DEPS_CFLAGS@ -I$(top_srcdir)/box2d/Include

lib_LTLIBRARIES = libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la

MARSHALFILES = clutter-box2d-marshal.c clutter-box2d-marshal.h

BUILT_SOURCES = $(MARSHALFILES)

libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LIBADD  = @DEPS_LIBS@ $(top_builddir)/box2d/libbox2d.la
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LDFLAGS = @CLUTTER_BOX2D_LT_LDFLAGS@
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_SOURCES = \
    clutter-box2d.cpp           \
    clutter-box2d-actor.cpp     \
    clutter-box2d-joint.cpp     \
    clutter-box2d-actor.h       \
    clutter-box2d-util.c        \
    clutter-box2d-collision.cpp \
    clutter-box2d-contact.cpp   \
    clutter-box2d-marshal.c

clutter_box2dheaders_HEADERS =  \
    clutter-box2d.h             \
    clutter-box2d-joint.h       \
    clutter-box2d-util.h        \
    clutter-box2d-collision.h   \
    clutter-box2d-contact.h     \
    clutter-box2d-marshal.h

clutter-box2d-marshal.c: clutter-box2d-marshal.list Makefile
	glib-genmarshal --prefix=clutter_box2d_marshal --body $< > $@

clutter-box2d-marshal.h: clutter-box2d-marshal.list Makefile
	glib-genmarshal --prefix=clutter_box2d_marshal --header $< > $@

clutter_box2dheadersdir = $(includedir)/clutter-0.9/clutter-box2d

DISTCLEANFILES = $(MARSHALFILES)

EXTRA_DIST = \
	clutter-box2d-marshal.list
