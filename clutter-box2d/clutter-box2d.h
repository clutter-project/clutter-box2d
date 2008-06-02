/*
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of box2d
 *
 * Copyright 2007, 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 */

#ifndef _CLUTTER_BOX2D_H
#define _CLUTTER_BOX2D_H

#include <clutter/clutter-group.h>

G_BEGIN_DECLS

/**
 * SECTION:clutter-box2d
 * @short_description: Container with physics engine
 *
 * ClutterBox2D is a container that can physically simulate collisions
 * between dynamic and static actors.
 */

#define CLUTTER_TYPE_BOX2D    clutter_box2d_get_type ()

#define CLUTTER_BOX2D(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                               CLUTTER_TYPE_BOX2D, ClutterBox2D))

#define CLUTTER_BOX2D_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
                            CLUTTER_TYPE_BOX2D, ClutterBox2DClass))

#define CLUTTER_IS_BOX2D(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                               CLUTTER_TYPE_BOX2D))

#define CLUTTER_IS_BOX2D_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                            CLUTTER_TYPE_BOX2D))

#define CLUTTER_BOX2D_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                              CLUTTER_TYPE_BOX2D, ClutterBox2DClass))

/**
 * ClutterBox2D:
 *
 * The #ClutterBox2D physics container, the struct has no public fields.
 */
typedef struct _ClutterBox2D        ClutterBox2D;
typedef struct _ClutterBox2DClass   ClutterBox2DClass;
typedef struct _ClutterBox2DPrivate ClutterBox2DPrivate;


struct _ClutterBox2D
{
  /*< private >*/
  ClutterGroup         parent_instance;
  ClutterBox2DPrivate *priv;

  void            *world;  /* The Box2D world which contains our simulation*/
  GHashTable      *actors; /* a hash table that maps actors to */
  GHashTable      *bodies; /* a hash table that maps bodies to */
  GHashTable      *joints;
};

struct _ClutterBox2DClass
{
  /*< private >*/
  ClutterGroupClass parent_class;
};

GType            clutter_box2d_get_type         (void) G_GNUC_CONST;

/**
 * ClutterBox2D:gravity
 *
 * The 2D vector specifying the gravity direction and magnitude, pass in 0.0,
 * 0.0 for zero gravity.
 */

/**
 * ClutterBox2D:simulating
 *
 * Whether the physics simulation engine is running or not.
 */


/**
 * clutter_box2d_new:
 *
 * Create a new #ClutterBox2D container.
 *
 * Returns: a new #ClutterBox2D container.
 */
ClutterActor *   clutter_box2d_new (void);

/**
 * clutter_box2d_set_simulating:
 * @box2d: a #ClutterBox2D
 * @simulating: the new state, TRUE or FALSE
 *
 * Sets whether the simulation engine of @box2d is running or not, the
 * value defaults to TRUE.
 */
void  clutter_box2d_set_simulating (ClutterBox2D *box2d,
                                    gboolean      simulating);

/**
 * clutter_box2d_get_simulating:
 * @box2d: a #ClutterBox2D
 *
 * Checks whether @box2d is simulating or not.
 * the simulation engine to be running when the group is created.
 *
 * Returns: whether the #ClutterBox2D group is currently doing physical simulation.
 */
gboolean  clutter_box2d_get_simulating (ClutterBox2D *box2d);

/**
 * clutter_box2d_set_gravity:
 * @box2d: a #ClutterBox2D
 * @gravity: the new gravity vector.
 *
 * Sets the direction and magnitude of gravity in the simulation as a 2D vector.
 */
void clutter_box2d_set_gravity (ClutterBox2D        *box2d,
                                const ClutterVertex *gravity);



/**
 * SECTION:clutter-box2d-actor
 * @short_description: Options for the children of ClutterBox2D
 *
 * The children of a ClutterBox2D container are accessed with both
 * the instance of ClutterBox2D and the child actor itself.
 */



/* <- not two dots, this not picked up by gtk-doc
 * ClutterBox2DActor:
 *
 * This structure contains the combined state of an actor and a body, all
 * actors added to the ClutterBox2D container have such ClutterBox2DActor
 * associated with it. The ClutterBox2DActor is an implementation detail
 * that is not exposed in the public API.
 */
typedef struct _ClutterBox2DActor ClutterBox2DActor;

/**
 * ClutterBox2DType:
 * @CLUTTER_BOX2D_NONE: No interaction
 * @CLUTTER_BOX2D_DYNAMIC: The actor is affected by collisions
 * @CLUTTER_BOX2D_STATIC: The actor affects collisions but is immobile
 *
 * Type of interactions between bodies.
 */
typedef enum { 
  CLUTTER_BOX2D_NONE = 0,
  CLUTTER_BOX2D_DYNAMIC,
  CLUTTER_BOX2D_STATIC,
} ClutterBox2DType;

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


/**
 * clutter_box2d_actor_set_linear_velocity:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @linear_velocity: the new linear velocity of the actor.
 *
 * Changes the linear velocity of the center of mass for an actor.
 */
void             clutter_box2d_actor_set_linear_velocity (ClutterBox2D *box2d,
                                                          ClutterActor *actor,
                                                          const ClutterVertex *linear_velocity);

/**
 * clutter_box2d_actor_get_linear_velocity:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @linear_velocity: pointer to a ClutterVertex for storing the current velocity.
 *
 * Retrieves the current linear velocity of a box2d governed actor.
 */
void             clutter_box2d_actor_get_linear_velocity (ClutterBox2D *box2d,
                                                          ClutterActor *actor,
                                                          ClutterVertex *linear_velocity);


/**
 * clutter_box2d_actor_set_angular_velocity:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @angular_velocity: the new angular velocity of the actor.
 *
 * Changes the angular velocity (speed of rotation) for an actor.
 */
void             clutter_box2d_actor_set_angular_velocity (ClutterBox2D *box2d,
                                                           ClutterActor *actor,
                                                           gdouble       angular_velocity);

/**
 * clutter_box2d_actor_get_angular_velocity:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 *
 * Query the current angular velocity (speed of rotation) for an actor.
 *
 * Returns: the current angular velocity for an actor.
 */
gdouble          clutter_box2d_actor_get_angular_velocity (ClutterBox2D *box2d,
                                                           ClutterActor *actor);


/**
 * clutter_box2d_actor_apply_force:
 * @box2d: a #ClutterBox2D
 * @actor: the actor to affect.
 * @force: the force vector to apply
 * @point: the point in @box2d coordinates to be affected.
 *
 * Applies a force to an actor at a specific point.
 */
void clutter_box2d_actor_apply_force         (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              ClutterVertex    *force,
                                              ClutterVertex    *point);

/**
 * clutter_box2d_actor_apply_impulse:
 * @box2d: a #ClutterBox2D
 * @actor: the actor to affect.
 * @force: the force vector to apply
 * @point: the point in @box2d coordinates to be affected.
 *
 * Applies an impulse to an actor at a specific point.
 */
void clutter_box2d_actor_apply_impulse       (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              ClutterVertex    *force,
                                              ClutterVertex    *point);


/**
 * clutter_box2d_actor_apply_torque:
 * @box2d: a #ClutterBox2D
 * @actor: the actor to affect.
 * @param torque about the z-axis (out of the screen), usually in N-m.
 *
 * Apply a torque. This affects the angular velocity
 * without affecting the linear velocity of the center of mass.
 */
void clutter_box2d_actor_apply_torque        (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gdouble           torque);





/**
 * clutter_box2d_actor_set_manipulatable:
 * @actor: a #ClutterActor in a ClutterBox2D container.
 *
 * Utility function that uses a mouse joint as well as mouse capture making
 * it possible to interact with the box2d simulation using the specified actor,
 * this call also sets @actor as reactive.
 */
void clutter_box2d_actor_set_manipulatable (ClutterActor *actor);


/**
 * clutter_box2d_get_actor:
 * @box2d: a #ClutterBox2D container.
 * @actor: a child of a @box2d
 *
 * Retrieve the meta object with child properties and other sync information.
 *
 * Returns: the meta object assoicated with @actor.
 */
ClutterBox2DActor *
clutter_box2d_get_actor (ClutterBox2D   *box2d,
                         ClutterActor   *actor);

G_END_DECLS

#endif

#include "clutter-box2d-joint.h"
