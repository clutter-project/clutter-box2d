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

#for a in `find box2d/Source -name '*.cpp'`;do echo $a \\ >> Makefile.am ;done

libclutter_box2d_@CLUTTER_BOX2D_MAJORMINOR@_la_SOURCES = clutter-box2d.cpp \
box2d/Source/Dynamics/Joints/b2MouseJoint.cpp \
box2d/Source/Dynamics/Joints/b2DistanceJoint.cpp \
box2d/Source/Dynamics/Joints/b2PulleyJoint.cpp \
box2d/Source/Dynamics/Joints/b2PrismaticJoint.cpp \
box2d/Source/Dynamics/Joints/b2GearJoint.cpp \
box2d/Source/Dynamics/Joints/b2Joint.cpp \
box2d/Source/Dynamics/Joints/b2RevoluteJoint.cpp \
box2d/Source/Dynamics/Contacts/b2Contact.cpp \
box2d/Source/Dynamics/Contacts/b2CircleContact.cpp \
box2d/Source/Dynamics/Contacts/b2PolyContact.cpp \
box2d/Source/Dynamics/Contacts/b2PolyAndCircleContact.cpp \
box2d/Source/Dynamics/Contacts/b2ContactSolver.cpp \
box2d/Source/Dynamics/b2Island.cpp \
box2d/Source/Dynamics/b2Body.cpp \
box2d/Source/Dynamics/b2ContactManager.cpp \
box2d/Source/Dynamics/b2World.cpp \
box2d/Source/Dynamics/b2WorldCallbacks.cpp \
box2d/Source/Common/b2BlockAllocator.cpp \
box2d/Source/Common/b2Settings.cpp \
box2d/Source/Common/b2StackAllocator.cpp \
box2d/Source/Common/b2Math.cpp \
box2d/Source/Collision/Shapes/b2Shape.cpp \
box2d/Source/Collision/Shapes/b2CircleShape.cpp \
box2d/Source/Collision/Shapes/b2PolygonShape.cpp \
box2d/Source/Collision/b2Distance.cpp \
box2d/Source/Collision/b2Collision.cpp \
box2d/Source/Collision/b2CollideCircle.cpp \
box2d/Source/Collision/b2CollidePoly.cpp \
box2d/Source/Collision/b2TimeOfImpact.cpp \
box2d/Source/Collision/b2PairManager.cpp \
box2d/Source/Collision/b2BroadPhase.cpp
