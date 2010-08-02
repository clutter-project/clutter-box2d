#ifndef _CLUTTER_BOX2D_JOINT_H
#define _CLUTTER_BOX2D_JOINT_H

G_BEGIN_DECLS

#include <clutter/clutter.h>
#include "clutter-box2d.h"

/*#include "Box2D.h"*/
/*#include "clutter-box2d-child.h"*/

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
 * Returns: a ClutterBox2DJoint handle or NULL on error.
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
 * Returns: a ClutterBox2DJoint handle or NULL on error.
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
 * Returns: a ClutterBox2DJoint handle or NULL on error.
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
 * Returns: a ClutterBox2DJoint handle or NULL on error.
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
 * Returns: a ClutterBox2DJoint handle or NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_mouse_joint (ClutterBox2D           *box2d,
                                                  ClutterActor           *actor,
                                                  const ClutterVertex    *target);


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
