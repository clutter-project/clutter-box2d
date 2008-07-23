SUBDIRS = . box2d clutter-box2d examples doc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

clutter-box2d-0.8.pc: clutter-box2d.pc
	@cp -f clutter-box2d.pc clutter-box2d-0.8.pc

pkgconfig_DATA = clutter-box2d-0.8.pc
pkgconfigdir   = $(libdir)/pkgconfig

INCLUDES = @GCC_FLAGS@ @DEPS_CFLAGS@ 

EXTRA_DIST = clutter-box2d.pc.in

CLEANFILES = clutter-box2d-0.8.pc
DISTCLEANFILES = clutter-box2d.pc

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in
