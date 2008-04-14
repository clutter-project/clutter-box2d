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
 * clutter_box2d_actor_get_type:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 *
 * Query the type of simulation performed on a child actor of a box2d group.
 *
 * Returns: the type of simulation performed for the actor, the default value
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


/**
 * clutter_box2d_actor_set_bullet:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 * @is_bullet: treat actor as bullet.
 *
 * Toggles whether actor is treated as a bullet (extra simulation steps to
 * avoid flying through thin objects at high speed.).
 */
void clutter_box2d_actor_set_bullet          (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gboolean          is_bullet);

/**
 * clutter_box2d_actor_is_bullet:
 * @box2d: a #ClutterBox2D
 * @actor: a ClutterActor that is a child of @box2d
 *
 * Queries whether an actor is considered a bullet.
 *
 * Returns: the current bullet state of the actor.
 */
gboolean clutter_box2d_actor_is_bullet           (ClutterBox2D     *box2d,
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

/*
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
 * Joints connects actors. Joints can be manually destroyed, if they are not
 * manually destroyed they are destroyed when one of the member actors of the
 * joint is destroyed.
 */

/**
 * ClutterBox2DJoint:
 *
 * A handle refering to a joint in a #ClutterBox2D container, joints are automatically
 * memory managed by Box2D and get destroyed if any of the actors invovled in the joint
 * is destroyed. You may also explicitly free the joint by calling #clutter_box2d_joint_destroy
 * on a joint that is no longer needed.
 */
typedef struct _ClutterBox2DJoint   ClutterBox2DJoint;

/**
 * clutter_box2d_add_revolute_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates fro the common point on @actor2
 * @reference_angle: the initial relative angle for joint limit (currently
 * unused)
 *
 * Create a revolute joint. A revolute joint defines a coordinates on two
 * actors that should coincide. The actors are allowed to rotate around this
 * point making it act like an axle.
 *
 * Returns: a ClutterBox2DJoint handle.
 */
ClutterBox2DJoint *clutter_box2d_add_revolute_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              reference_angle);


/**
 * clutter_box2d_add_revolute_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor: the world (box2d container) coordinates for the point that @actor1
 * and @actor2 are allowed to revolve around.
 *
 * Create a revolute joint that is defined by original positions of actors and
 * a common point specified in world coordinates.
 *
 * Returns: a ClutterBox2DJoint handle.
 */
ClutterBox2DJoint *clutter_box2d_add_revolute_joint2 (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor);


/**
 * clutter_box2d_add_distance_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates fro the common point on @actor2
 * @length: the length that the simulation will maintain between anchor1 on
 * actor1 and anchor2 on actor2.
 * @frequency: the frequency of length updates.
 * @damping_ratio: the damping ratio.
 *
 * A distance joint constrains two points on two bodies to remain at a fixed
 * distance from each other. You can view this as a massless, rigid rod. By
 * modifying @frequency and @damping_ratio you can achieve a spring like
 * behavior as well. The defaults for frequency and damping_ratio to disable
 * dampening is 0.0 for both.
 *
 * Returns: a ClutterBox2DJoint handle.
 */
ClutterBox2DJoint *clutter_box2d_add_distance_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              length,
                                                     gdouble              frequency,
                                                     gdouble              damping_ratio);


/**
 * clutter_box2d_add_prismatic_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates fro the common point on @actor2
 * @min_length: minimum distance between anchor points
 * @max_length: maximum distance between anchor points.
 * @axis: the local translation axis in @body1.
 *
 * A prismatic joint. This joint provides one degree of freedom: translation
 * along an axis fixed in body1. Relative rotation is prevented.
 *
 * Returns: a ClutterBox2DJoint handle.
 */
ClutterBox2DJoint *clutter_box2d_add_prismatic_joint (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor1,
                                                      const ClutterVertex *anchor2,
                                                      gdouble              min_length,
                                                      gdouble              max_length,
                                                      const ClutterVertex *axis);


/**
 * clutter_box2d_add_mouse_joint:
 * @box2d: a #ClutterBox2D
 * @actor: the (dynamic) actor to be manipulated.
 * @target: the box2d container coordinates of the mouse.
 *
 * A mouse joint is used to make a point on a dynamic actor track a specified
 * world point. This a soft constraint with a maximum force. This allows the
 * constraint to stretch and without applying huge forces.
 *
 * Returns: a ClutterBox2DJoint handle.
 */
ClutterBox2DJoint *clutter_box2d_add_mouse_joint (ClutterBox2D     *box2d,
                                                  ClutterActor     *actor,
                                                  ClutterVertex    *target);


/**
 * clutter_box2d_mouse_joint_update_target:
 * @mouse_joint: A #ClutterBox2DJoint priorly returned from #clutter_box2d_add_mouse_joint.
 * @target: new box2d container coordinates for mouse pointer.
 *
 * Updates the position the the target point should coincide with. By updating this
 * in a motion event callback for mouse motion physical interaction with dynamic actors
 * is possible.
 */
void clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *mouse_joint,
                                              const ClutterVertex *target);

/**
 * clutter_box2d_joint_destroy:
 * @joint: A #ClutterBox2DJoint
 *
 * Destroys a #ClutterBox2DJoint, call this function manually to remove a joint
 * that you no longer have need for. Note that it is mostly not neccesary to
 * destroy joints that are part of models manually since they will be destroyed
 * automatically when the actors they use are destroyed.
 */
void clutter_box2d_joint_destroy             (ClutterBox2DJoint   *joint);



G_END_DECLS

#endif
