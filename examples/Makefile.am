noinst_PROGRAMS = blockbox

INCLUDES = -I$(top_srcdir)/ @GCC_FLAGS@ @DEPS_CFLAGS@
LDFLAGS = @DEPS_LIBS@ $(top_builddir)/libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la 

blockbox_SOURCES = \
	blockbox.c		\
	actor-manipulator.c	\
	actor-manipulator.h	\
	popup.c			\
	popup.h			\
	scenes.h		\
	scenes.c		\
	util.c			\
	util.h			\
	wrap-group.c		\
	wrap-group.h		\
	label-action.c		\
	label-action.h		\
	\
	scene-pyramid.c		\
	scene-slides.c 		\
	scene-control.c 	\
	scene-bridge.c 		\
	scene-chain.c 		\
	scene-distance-joint.c 	\
	scene-prismatic-joint.c \
	scene-about.c

test: blockbox
	./blockbox
