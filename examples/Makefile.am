noinst_PROGRAMS = blockbox

INCLUDES = -I$(top_srcdir)/ @GCC_FLAGS@ @DEPS_CFLAGS@
LDFLAGS = @DEPS_LIBS@ $(top_builddir)/libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la 

blockbox_SOURCES = blockbox.c

