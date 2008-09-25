/**
 * This file provides the declaration for the 
 * ClutterBox2DCollision object passed to the signal handlers for the 
 * "collision" signal found on ClutterBox2DActor.
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * Original code authored by James Ketrenos <jketreno@linux.intel.com>
 *
 */
#ifndef __clutter_box2d_collision_h__
#define __clutter_box2d_collision_h__

G_BEGIN_DECLS

#define CLUTTER_TYPE_BOX2D_COLLISION      (clutter_box2d_collision_get_type ())

#define CLUTTER_BOX2D_COLLISION(obj) \
 (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLUTTER_TYPE_BOX2D_COLLISION, ClutterBox2DCollision))
#define CLUTTER_BOX2D_COLLISION_CLASS(klass) \
 (G_TYPE_CHECK_CLASS_CAST ((klass), CLUTTER_TYPE_BOX2D_COLLISION, ClutterBox2DCollisionaClass))
#define CLUTTER_IS_BOX2D_COLLISION(obj) \
 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLUTTER_TYPE_BOX2D_COLLISION))
#define CLUTTER_IS_BOX2D_COLLISION_CLASS(klass) \
 (G_TYPE_CHECK_CLASS_TYPE ((klass), CLUTTER_TYPE_BOX2D_COLLISION))
#define CLUTTER_BOX2D_COLLISION_GET_CLASS(obj) \
 (G_TYPE_INSTANCE_GET_CLASS ((obj), CLUTTER_TYPE_BOX2D_COLLISION, ClutterBox2DCollisionClass))

typedef struct _ClutterBox2DCollision ClutterBox2DCollision;
typedef struct _ClutterBox2DCollisionClass ClutterBox2DCollisionClass;
typedef struct _ClutterBox2DCollisionPrivate ClutterBox2DCollisionPrivate;

GType clutter_box2d_collision_get_type (void) G_GNUC_CONST;

/**
 * ClutterBox2DCollision:
 * @actor1        Body 1 in collision
 * @actor2        Body 2 in collision
 *
 * Contact point information associated with a collision in Box2D.
 *
 * NOTE:  The signal callback "collision" emitted by ClutterBox2DActor
 * provides the ClutterBox2DActor as the instance (which is the Box2D actor
 * which shadows the ClutterActor typically referenced by the calling 
 * application)  As such, the caller code should typically use the
 * ClutterActor obtained through @actor1 to reference the "real" ClutterActor
 * involved in the collision.
 *
 */
struct _ClutterBox2DCollision
{
  /*< private >*/
  GObject parent_instance;

  ClutterActor   *actor1;       /* Actor 1 in collision */
  ClutterActor   *actor2;       /* Actor 2 in collision */
  ClutterVertex  position;      /* Box2D world coordinates for collision point */
  ClutterVertex  normal;        /* Unit vector pointing from Actor1 to Actor2 */
  gdouble        normal_force;  /* Contact solver's calculated collision 
				 * intensity */
  gdouble        tangent_force; /* Box2D contact solver estimate of friction 
				 * force */
  gulong         id;            /* Contact ID */
};

struct _ClutterBox2DCollisionClass
{
  GObjectClass parent_class;
};

typedef void (*ClutterBox2DCollisionHandler) (
  ClutterActor          *actor,
  ClutterBox2DCollision *collision,
  gpointer               data);

G_END_DECLS

#endif
