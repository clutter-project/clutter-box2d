noinst_PROGRAMS = blockbox

INCLUDES = -I$(top_srcdir)/clutter-box2d
AM_CPPFLAGS = @MAINTAINER_CFLAGS@ @DEPS_CFLAGS@
LDADD   = \
	$(top_builddir)/clutter-box2d/libclutter-box2d-@CLUTTER_BOX2D_API_VERSION@.la \
	@DEPS_LIBS@

EXTRA_DIST = redhand.png

blockbox_SOURCES = \
	blockbox.c		\
	blockbox.h		\
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
	scene-table.c 		\
	scene-distance-joint.c 	\
	scene-prismatic-joint.c \
	scene-pulley-joint.c 	\
	scene-car.c 		\
	scene-about.c
