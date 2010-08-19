#ifndef _CLUTTER_BOX2D_JOINT_H
#define _CLUTTER_BOX2D_JOINT_H

G_BEGIN_DECLS

#include <clutter/clutter.h>
#include <clutter-box2d/clutter-box2d.h>

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
 * ClutterBox2DJointType:
 * @CLUTTER_BOX2D_JOINT_DEAD: The joint has become invalid
 * @CLUTTER_BOX2D_JOINT_DISTANCE: A distance joint
 * @CLUTTER_BOX2D_JOINT_PRISMATIC: A prismatic joint
 * @CLUTTER_BOX2D_JOINT_LINE: A line joint
 * @CLUTTER_BOX2D_JOINT_REVOLUTE: A revolute joint
 * @CLUTTER_BOX2D_JOINT_PULLEY: A pulley joint
 * @CLUTTER_BOX2D_JOINT_WELD: A weld joint
 * @CLUTTER_BOX2D_JOINT_MOUSE: A mouse joint
 *
 * Identifiers for different joint types.
 */
typedef enum
{
  CLUTTER_BOX2D_JOINT_DEAD,
  CLUTTER_BOX2D_JOINT_DISTANCE,
  CLUTTER_BOX2D_JOINT_PRISMATIC,
  CLUTTER_BOX2D_JOINT_LINE,
  CLUTTER_BOX2D_JOINT_REVOLUTE,
  CLUTTER_BOX2D_JOINT_PULLEY,
  CLUTTER_BOX2D_JOINT_WELD,
  CLUTTER_BOX2D_JOINT_MOUSE
} ClutterBox2DJointType;

/**
 * clutter_box2d_joint_get_type:
 * @joint: A #ClutterBox2DJoint
 *
 * Retrieves the type of the joint.
 *
 * Returns: a #ClutterBox2DJointType
 */
ClutterBox2DJointType
clutter_box2d_joint_get_type (ClutterBox2DJoint *joint);

/**
 * clutter_box2d_add_revolute_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates fro the common point on @actor2
 *
 * Create a revolute joint. A revolute joint defines a coordinates on two
 * actors that should coincide. The actors are allowed to rotate around this
 * point making it act like an axle.
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_revolute_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2);


/**
 * clutter_box2d_add_revolute_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor: the world (box2d container) coordinates for the point that @actor1
 * and @actor2 are allowed to revolve around.
 *
 * Convenience function for creativing a revolute joint using world coordinates.
 * See clutter_box2d_add_revolute_joint().
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
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
 * @anchor1: the local coordinates for the anchor point on @actor1
 * @anchor2: the local coordinates for the anchor point on @actor2
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
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
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
 * clutter_box2d_add_distance_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the world coordinates for the anchor point on @actor1
 * @anchor2: the world coordinates for the anchor point on @actor2
 * @frequency: the frequency of length updates.
 * @damping_ratio: the damping ratio.
 *
 * Convenience function for specifying a distance joint with world coordinates.
 * See clutter_box2d_add_distance_joint().
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_distance_joint2 (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor1,
                                                      const ClutterVertex *anchor2,
                                                      gdouble              frequency,
                                                      gdouble              damping_ratio);


/**
 * clutter_box2d_add_prismatic_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates for the common point on @actor2
 * @min_length: minimum distance between anchor points
 * @max_length: maximum distance between anchor points.
 * @axis: the local translation axis in @body1.
 *
 * A prismatic joint. This joint provides one degree of freedom: translation
 * along an axis fixed in body1. Relative rotation is prevented.
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
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
 * clutter_box2d_add_prismatic_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor: the world coordinates for the joint anchor point
 * @min_length: minimum distance between anchor points
 * @max_length: maximum distance between anchor points.
 * @axis: the local translation axis in @body1.
 *
 * Convenience function for creating a prismatic joint using world coordinates.
 * See clutter_box2d_add_prismatic_joint().
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_prismatic_joint2 (ClutterBox2D        *box2d,
                                                       ClutterActor        *actor1,
                                                       ClutterActor        *actor2,
                                                       const ClutterVertex *anchor,
                                                       gdouble              min_length,
                                                       gdouble              max_length,
                                                       const ClutterVertex *axis);

/**
 * clutter_box2d_add_line_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates for the common point on @actor2
 * @min_length: minimum distance between anchor points
 * @max_length: maximum distance between anchor points.
 * @axis: the local translation axis in @body1.
 *
 * This creates a line joint. A line joint is the same as a prismatic joint,
 * with the rotation constraint removed. This allows for creating things like
 * vehicle suspensions. See clutter_box2d_add_prismatic_joint().
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_line_joint (ClutterBox2D        *box2d,
                                                 ClutterActor        *actor1,
                                                 ClutterActor        *actor2,
                                                 const ClutterVertex *anchor1,
                                                 const ClutterVertex *anchor2,
                                                 gdouble              min_length,
                                                 gdouble              max_length,
                                                 const ClutterVertex *axis);

/**
 * clutter_box2d_add_line_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in joint
 * @actor2: second actor participating in joint
 * @anchor: the world coordinates for the joint anchor point
 * @min_length: minimum distance between anchor points
 * @max_length: maximum distance between anchor points.
 * @axis: the local translation axis in @body1.
 *
 * Convenience function for creating a line joint with world coordinates.
 * See clutter_box2d_add_line_joint().
 *
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_line_joint2 (ClutterBox2D        *box2d,
                                                  ClutterActor        *actor1,
                                                  ClutterActor        *actor2,
                                                  const ClutterVertex *anchor,
                                                  gdouble              min_length,
                                                  gdouble              max_length,
                                                  const ClutterVertex *axis);

/**
 * clutter_box2d_add_pulley_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in the joint
 * @actor2: second actor participating in the joint
 * @anchor1: the local coordinates for the anchor point on @actor1
 * @anchor2: the local coordinates for the anchor point on @actor2
 * @ground_anchor1: the world coordinates for the ground anchor point of @actor1
 * @ground_anchor2: the world coordinates for the ground anchor point of @actor2
 * @length1: The initial length of the pulley rope for @actor1
 * @length2: The initial length of the pulley rope for @actor2
 * @max_length1: The maximum length of the pulley rope for @actor1
 * @max_length2: The maximum length of the pulley rope for @actor2
 * @ratio: The pulley ratio between @actor1 and @actor2
 *
 * A pulley joint is used to create an idealized pulley. It connects two actors
 * to each other via a virtual rope, extending from two 'ground' points in the
 * world. The length of the rope is conserved according to the initial
 * configuration. You can supply a ratio that simulates a block and tackle. This
 * can be used to create mechanical leverage. For example, if @ratio is 2, the
 * length of the rope suspending @actor1 will vary at twice the rate of that
 * suspending @actor2.
 *
 * @max_length1 and @max_length2 are provided to increase stability, as the
 * constraint equations become singular when either side's rope is of length
 * zero. They can also be used for gameplay reasons.
 *
 * Returns: a #ClutterBox2DJoint handle, or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_pulley_joint (ClutterBox2D        *box2d,
                                                   ClutterActor        *actor1,
                                                   ClutterActor        *actor2,
                                                   const ClutterVertex *anchor1,
                                                   const ClutterVertex *anchor2,
                                                   const ClutterVertex *ground_anchor1,
                                                   const ClutterVertex *ground_anchor2,
                                                   gdouble              length1,
                                                   gdouble              length2,
                                                   gdouble              max_length1,
                                                   gdouble              max_length2,
                                                   gdouble              ratio);

/**
 * clutter_box2d_add_pulley_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in the joint
 * @actor2: second actor participating in the joint
 * @anchor1: the world coordinates for the anchor point on @actor1
 * @anchor2: the world coordinates for the anchor point on @actor2
 * @ground_anchor1: the world coordinates for the ground anchor point of @actor1
 * @ground_anchor2: the world coordinates for the ground anchor point of @actor2
 * @ratio: The pulley ratio between @actor1 and @actor2
 *
 * Convenience function for creating a pulley joint, using world coordinates.
 * See clutter_box2d_add_pulley_joint().
 *
 * Returns: a #ClutterBox2DJoint handle, or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_pulley_joint2 (ClutterBox2D        *box2d,
                                                    ClutterActor        *actor1,
                                                    ClutterActor        *actor2,
                                                    const ClutterVertex *anchor1,
                                                    const ClutterVertex *anchor2,
                                                    const ClutterVertex *ground_anchor1,
                                                    const ClutterVertex *ground_anchor2,
                                                    gdouble              ratio);

/**
 * clutter_box2d_add_weld_joint:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in the joint
 * @actor2: second actor participating in the joint
 * @anchor1: the local coordinates for the common point on @actor1
 * @anchor2: the local coordinates for the common point on @actor2
 *
 * A weld joint is used to constrain all relative motion between two bodies.
 * Note that this should not be used as an alternative to composite actors,
 * as the joint allows a small amount of give due to error.
 *
 * Returns: a #ClutterBox2DJoint handle, or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_weld_joint (ClutterBox2D        *box2d,
                                                 ClutterActor        *actor1,
                                                 ClutterActor        *actor2,
                                                 const ClutterVertex *anchor1,
                                                 const ClutterVertex *anchor2);

/**
 * clutter_box2d_add_weld_joint2:
 * @box2d: a #ClutterBox2D
 * @actor1: first actor participating in the joint
 * @actor2: second actor participating in the joint
 * @anchor: the world coordinates of the anchor point on both actors
 *
 * Convenience function to create a weld joint with world coordinates.
 * See clutter_box2d_add_weld_joint().
 *
 * Returns: a #ClutterBox2DJoint handle, or %NULL on error.
 */
ClutterBox2DJoint *clutter_box2d_add_weld_joint2 (ClutterBox2D        *box2d,
                                                  ClutterActor        *actor1,
                                                  ClutterActor        *actor2,
                                                  const ClutterVertex *anchor);

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
 * Returns: a #ClutterBox2DJoint handle or %NULL on error.
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

/**
 * clutter_box2d_joint_set_engine:
 * @joint: A #ClutterBox2DJoint
 * @enable: %TRUE to enable the engine, %FALSE to disable
 * @max_force: The maximum motor force, or torque, in N
 * @speed: Motor speed, in radians per second
 *
 * Enables or disables an engine in the specified joint. This is only valid for
 * revolute, prismatic and line joints. This function does nothing for joints
 * that don't support engines.
 *
 * Force (or torque, in the case of a revolute joint) is measured in Newtons,
 * but what this means will depend on the given size of world-units (the scale-
 * factor) and the density of the body in question.
 */
void clutter_box2d_joint_set_engine (ClutterBox2DJoint *joint,
                                     gboolean           enable,
                                     gdouble            max_force,
                                     gdouble            speed);

G_END_DECLS

#endif
