noinst_LTLIBRARIES = libbox2d.la

INCLUDES = @GCC_FLAGS@ -ISource -I. -IInclude

EXTRA_DIST = Readme.txt License.txt

#for a in `find Source -name '*.cpp' -or -name '*.h'`; do echo $a \\ >> Makefile.am ;done
libbox2d_la_SOURCES = Include/Box2D.h \
Source/Dynamics/Joints/b2MouseJoint.cpp \
Source/Dynamics/Joints/b2DistanceJoint.cpp \
Source/Dynamics/Joints/b2PulleyJoint.cpp \
Source/Dynamics/Joints/b2RevoluteJoint.h \
Source/Dynamics/Joints/b2PrismaticJoint.cpp \
Source/Dynamics/Joints/b2MouseJoint.h \
Source/Dynamics/Joints/b2GearJoint.cpp \
Source/Dynamics/Joints/b2DistanceJoint.h \
Source/Dynamics/Joints/b2LineJoint.h \
Source/Dynamics/Joints/b2LineJoint.cpp \
Source/Dynamics/Joints/b2Joint.cpp \
Source/Dynamics/Joints/b2PulleyJoint.h \
Source/Dynamics/Joints/b2PrismaticJoint.h \
Source/Dynamics/Joints/b2GearJoint.h \
Source/Dynamics/Joints/b2Joint.h \
Source/Dynamics/Joints/b2RevoluteJoint.cpp \
Source/Dynamics/Contacts/b2Contact.cpp \
Source/Dynamics/Contacts/b2ContactSolver.h \
Source/Dynamics/Contacts/b2NullContact.h \
Source/Dynamics/Contacts/b2CircleContact.cpp \
Source/Dynamics/Contacts/b2PolyContact.cpp \
Source/Dynamics/Contacts/b2Contact.h \
Source/Dynamics/Contacts/b2CircleContact.h \
Source/Dynamics/Contacts/b2PolyAndCircleContact.cpp \
Source/Dynamics/Contacts/b2PolyContact.h \
Source/Dynamics/Contacts/b2ContactSolver.cpp \
Source/Dynamics/Contacts/b2PolyAndCircleContact.h \
Source/Dynamics/b2Island.cpp \
Source/Dynamics/b2ContactManager.h \
Source/Dynamics/b2World.h \
Source/Dynamics/b2WorldCallbacks.h \
Source/Dynamics/b2Body.cpp \
Source/Dynamics/b2Island.h \
Source/Dynamics/b2Body.h \
Source/Dynamics/b2ContactManager.cpp \
Source/Dynamics/b2World.cpp \
Source/Dynamics/b2WorldCallbacks.cpp \
Source/Common/b2BlockAllocator.cpp \
Source/Common/b2StackAllocator.h \
Source/Common/b2Math.h \
Source/Common/jtypes.h \
Source/Common/Fixed.h \
Source/Common/b2BlockAllocator.h \
Source/Common/b2Settings.cpp \
Source/Common/b2Settings.h \
Source/Common/b2StackAllocator.cpp \
Source/Common/b2Math.cpp \
Source/Collision/Shapes/b2PolygonShape.h \
Source/Collision/Shapes/b2Shape.cpp \
Source/Collision/Shapes/b2CircleShape.cpp \
Source/Collision/Shapes/b2Shape.h \
Source/Collision/Shapes/b2CircleShape.h \
Source/Collision/Shapes/b2PolygonShape.cpp \
Source/Collision/b2Distance.cpp \
Source/Collision/b2Collision.cpp \
Source/Collision/b2PairManager.h \
Source/Collision/b2BroadPhase.h \
Source/Collision/b2CollideCircle.cpp \
Source/Collision/b2CollidePoly.cpp \
Source/Collision/b2Collision.h \
Source/Collision/b2TimeOfImpact.cpp \
Source/Collision/b2PairManager.cpp \
Source/Collision/b2BroadPhase.cpp

