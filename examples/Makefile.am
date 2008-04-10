noinst_PROGRAMS = blockbox

INCLUDES = -I$(top_srcdir)/clutter-box2d @GCC_FLAGS@ @DEPS_CFLAGS@
AM_LDFLAGS = @DEPS_LIBS@ $(top_builddir)/clutter-box2d/libclutter-box2d-@CLUTTER_BOX2D_MAJORMINOR@.la 

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
