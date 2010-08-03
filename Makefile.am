ACLOCAL_AMFLAGS = -I build/autotools

SUBDIRS = build box2d clutter-box2d examples doc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

clutter-box2d-@CLUTTER_BOX2D_API_VERSION@.pc: clutter-box2d.pc
	$(AM_V_GEN)cp -f clutter-box2d.pc clutter-box2d-@CLUTTER_BOX2D_API_VERSION@.pc

pkgconfig_DATA = clutter-box2d-@CLUTTER_BOX2D_API_VERSION@.pc
pkgconfigdir   = $(libdir)/pkgconfig

EXTRA_DIST = clutter-box2d.pc.in

CLEANFILES = clutter-box2d-@CLUTTER_BOX2D_API_VERSION@.pc
DISTCLEANFILES = clutter-box2d.pc

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in
