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
#include <clutter/clutter-child-meta.h>
#include "clutter-box2d-actor.h"
#include "clutter-box2d.h"
#include "math.h"

G_BEGIN_DECLS

/**
 * SECTION:clutter-box2d
 * @short_description: Container with physics engine
 *
 * ClutterBox2D is a container that can physically simulate collisions
 * between dynamic and static actors.
 */

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

/**
 * ClutterBox2DActor:
 *
 * The #ClutterBox2DActor physics container, the struct has no public fields.
 */
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

  b2Body           *body;  /* Box2D body, if any */
  b2Shape          *shape; /* shape attached to this body, if any */
  GList            *joints;  /* list of joints this body participates in */
  b2World          *world;/*the Bod2D world (could be looked up through box2d)*/

};

struct _ClutterBox2DActorClass
{
  /*< private >*/
  ClutterChildMetaClass parent_class;
};

GType   clutter_box2d_actor_get_type  (void) G_GNUC_CONST;


/**
 * clutter_box2d_actor_set_type:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @type: the new simulation mode of @actor.
 *
 * Changes the type of simulation performed on a clutter actor.
 */
void  clutter_box2d_actor_set_type (ClutterBox2D     *box2d,
                                    ClutterActor     *actor,
                                    ClutterBox2DType  type);

/**
 * clutter_box2d_actor_get_type2:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @type: the new simulation mode of @actor.
 *
 * Changes the type of simulation performed on a clutter actor.
 */

ClutterBox2DType
clutter_box2d_actor_get_type2 (ClutterBox2D     *box2d,
                               ClutterActor     *actor);



void clutter_box2d_actor_set_bullet          (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gboolean          is_bullet);

gboolean clutter_box2d_actor_is_bullet       (ClutterBox2D     *box2d,
                                              ClutterActor     *actor);

void sync_body  (ClutterBox2DActor *box2d_actor);
void sync_actor (ClutterBox2DActor *box2d_actor);

G_END_DECLS

#endif
