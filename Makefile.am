SUBDIRS = . examples

clutter-box2d-0.6.pc: clutter-box2d.pc
	@cp -f clutter-box2d.pc clutter-box2d-0.6.pc

pkgconfig_DATA = clutter-box2d-0.6.pc
pkgconfigdir   = $(libdir)/pkgconfig

INCLUDES = @GCC_FLAGS@ @DEPS_CFLAGS@ 

lib_LTLIBRARIES = libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la

libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_SOURCES = clutter-box2d.cpp
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LIBADD  = @DEPS_LIBS@
libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_LDFLAGS = @CLUTTER_BOX2D_LT_LDFLAGS@

clutter_box2dheaders_HEADERS = clutter-box2d.h
clutter_box2dheadersdir = $(includedir)/clutter-0.6/clutter-box2d

EXTRA_DIST = clutter-box2d.pc.in

CLEANFILES = clutter-box2d-0.6.pc
DISTCLEANFILES = clutter-box2d.pc

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in
