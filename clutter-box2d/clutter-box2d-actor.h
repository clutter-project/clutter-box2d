/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DActor class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#ifndef _CLUTTER_BOX2D_ACTOR_H
#define _CLUTTER_BOX2D_ACTOR_H

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d-actor.h"
#include "clutter-box2d.h"
#include "math.h"

G_BEGIN_DECLS

#define CLUTTER_TYPE_BOX2D_ACTOR    clutter_box2d_actor_get_type ()

#define CLUTTER_BOX2D_ACTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                               CLUTTER_TYPE_BOX2D_ACTOR, ClutterBox2DActor))

#define CLUTTER_BOX2D_ACTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
                            CLUTTER_TYPE_BOX2D_ACTOR, ClutterBox2DActorClass))

#define CLUTTER_IS_BOX2D_ACTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                               CLUTTER_TYPE_BOX2D_ACTOR))

#define CLUTTER_IS_BOX2D_ACTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                            CLUTTER_TYPE_BOX2D_ACTOR))

#define CLUTTER_BOX2D_ACTOR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                              CLUTTER_TYPE_BOX2D_ACTOR, ClutterBox2DActorClass))

typedef struct _ClutterBox2DActor        ClutterBox2DActor;
typedef struct _ClutterBox2DActorPrivate ClutterBox2DActorPrivate;
typedef struct _ClutterBox2DActorClass   ClutterBox2DActorClass;

struct _ClutterBox2DActor
{
  /*< private >*/
  ClutterChildMeta          parent_instance;
  ClutterBox2DActorPrivate *priv; /* not used anymore */

  ClutterBox2DType  type; /* The type Static: the body affects collisions but
                             is not itself affected. Dynamic: the body is
                             affected by collisions.*/ 
  gboolean          is_circle;
  ClutterVertex    *outline;

  b2Body           *body;   /* Box2D body, if any */
  b2Shape          *shape;  /* shape attached to this body, if any */
  GList            *joints; /* list of joints this body participates in */
  b2World          *world;  /*the Box2D world (could be looked up through box2d)*/

  gfloat            density;
  gfloat            friction;
  gfloat            restitution;
};

struct _ClutterBox2DActorClass
{
  /*< private >*/
  ClutterChildMetaClass parent_class;
};

ClutterBox2DActor * clutter_box2d_get_actor (ClutterBox2D   *box2d,
                                             ClutterActor   *actor);

GType   clutter_box2d_actor_get_type  (void) G_GNUC_CONST;

G_END_DECLS

#endif
