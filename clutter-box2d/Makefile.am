INCLUDES = @GCC_FLAGS@ @DEPS_CFLAGS@ -I$(top_srcdir)/box2d/Include

lib_LTLIBRARIES = libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la

libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LIBADD  = @DEPS_LIBS@ $(top_builddir)/box2d/libbox2d.la
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LDFLAGS = @CLUTTER_BOX2D_LT_LDFLAGS@
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_SOURCES = clutter-box2d.cpp

clutter_box2dheaders_HEADERS = clutter-box2d.h
clutter_box2dheadersdir = $(includedir)/clutter-0.7/clutter-box2d
