/*
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of box2d
 *
 * Copyright 2007 OpenedHand Ltd
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

typedef struct _ClutterBox2D        ClutterBox2D;
typedef struct _ClutterBox2DClass   ClutterBox2DClass;
typedef struct _ClutterBox2DPrivate ClutterBox2DPrivate;
typedef struct _ClutterBox2DJoint   ClutterBox2DJoint;

struct _ClutterBox2D
{
  /*< private >*/
  ClutterGroup         parent_instance;
  ClutterBox2DPrivate *priv;
};

struct _ClutterBox2DClass
{
  /*< private >*/
  ClutterGroupClass parent_class;
};

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

GType            clutter_box2d_get_type         (void) G_GNUC_CONST;


/**
 * clutter_box2d_new:
 *
 * Returns a new #ClutterBox2D container.
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

/* This structure contains the combined state of an actor and a body, all
 * actors added to the ClutterBox2D container have such ClutterBox2DActor
 * associated with it.
 */
typedef struct _ClutterBox2DActor ClutterBox2DActor;

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
 * clutter_box2d_actor_get_type:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 *
 * Returns the type of simulation performed for the actor, the default value
 * if no simulation has been set is CLUTTER_BOX2D_NONE.
 */
ClutterBox2DType clutter_box2d_actor_get_type (ClutterBox2D *box2d,
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


void clutter_box2d_actor_set_bullet          (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gboolean          is_bullet);

gboolean clutter_box2d_actor_is_bullet           (ClutterBox2D     *box2d,
                                                  ClutterActor     *actor);
/*
void             clutter_box2d_actor_set_linear_velocity (ClutterBox2D *box2d,
                                                          ClutterActor *actor,
                                                          const ClutterVertex *linear_velocity);

void             clutter_box2d_actor_get_linear_velocity (ClutterBox2D *box2d,
                                                          ClutterActor *actor,
                                                          ClutterVertex *linear_velocity);

void             clutter_box2d_actor_set_angular_velocity (ClutterBox2D *box2d,
                                                           ClutterActor *actor,
                                                           gdouble       omega);
gdouble          clutter_box2d_actor_get_angular_velocity (ClutterBox2D *box2d,
                                                           ClutterActor *actor);


void clutter_box2d_actor_apply_impulse       (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              ClutterVertex    *force,
                                              ClutterVertex    *point);

void clutter_box2d_actor_apply_torque        (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gdouble           torque);


*/


/**
 * SECTION:clutter-box2d-joint
 * @short_description: Joint creation and manipulation.
 *
 * Joints connects actor, both dynamic and static.
 */

ClutterBox2DJoint *clutter_box2d_add_revolute_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              reference_angle);


/* This call takes a single anchor point, which is in world, not local actor
 * coordinates
 */ 
ClutterBox2DJoint *clutter_box2d_add_revolute_joint2 (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor);


ClutterBox2DJoint *clutter_box2d_add_distance_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              length,
                                                     gdouble              frequency,
                                                     gdouble              damping_ratio);


ClutterBox2DJoint *clutter_box2d_add_prismatic_joint (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor1,
                                                      const ClutterVertex *anchor2,
                                                      gdouble              min_length,
                                                      gdouble              max_length,
                                                      const ClutterVertex *axis);

ClutterBox2DJoint *clutter_box2d_add_mouse_joint (ClutterBox2D     *box2d,
                                                  ClutterActor     *actor,
                                                  ClutterVertex    *target);

void clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *mouse_joint,
                                              const ClutterVertex *target);

void clutter_box2d_joint_destroy             (ClutterBox2DJoint   *joint);



G_END_DECLS

#endif
