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

BUILT_GIRSOURCES =

ClutterBox2D-@CLUTTER_BOX2D_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libclutter-box2d-0.9.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace ClutterBox2D --nsversion=@CLUTTER_BOX2D_API_VERSION@ \
		--add-include-path=$(srcdir) --add-include=path=. \
		--include=Clutter-@CLUTTER_BOX2D_API_VERSION@ \
		--library=clutter-box2d-@CLUTTER_BOX2D_API_VERSION@ \
		--output $@ \
		--pkg gobject-2.0 \
		--pkg clutter-@CLUTTER_BOX2D_API_VERSION@ \
		clutter-box2d.h clutter-box2d-joint.h clutter-box2d-util.h

BUILT_GIRSOURCES += ClutterBox2D-@CLUTTER_BOX2D_API_VERSION@.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES=
CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
