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

#include <clutter/clutter.h>

/* Abstract declaration of ClutterBox2DContactListener */
typedef struct _ClutterBox2DContactListener ClutterBox2DContactListener;

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

  /*< public >*/
  void (* iterate) (ClutterBox2D *box2d);
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
 * ClutterBox2D:scale-factor
 *
 * The scaling factor of pixels to world units. For example, if you consider
 * world units to be metres, setting a scale-factor of 1/50 would make 50
 * pixels equivalent to 1 metre.
 */

/**
 * ClutterBox2D:time-step
 *
 * The amount of time simulated between physics steps. Lower values tend to
 * yield a higher-quality. 60fps (i.e. 1000/60) is the default and recomended
 * value.
 */

/**
 * ClutterBox2D:iterations
 *
 * The amount of iterations to perform on each physics step to resolve
 * contacts and joints. Larger values yield a more accurate simulation,
 * at the cost of CPU usage.
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
 * clutter_box2d_set_gravity:
 * @box2d: a #ClutterBox2D
 * @gravity: The gravity vector
 *
 * Sets the world's gravity on the x and y axes. The z axis is ignored.
 */
void  clutter_box2d_set_gravity (ClutterBox2D        *box2d,
                                 const ClutterVertex *gravity);

/**
 * clutter_box2d_get_gravity:
 * @box2d: a #ClutterBox2D
 * @gravity: a #ClutterVertex
 *
 * Retrieves the world's gravity vector and puts it in @gravity.
 */
void  clutter_box2d_get_gravity (ClutterBox2D  *box2d,
                                 ClutterVertex *gravity);

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
 * clutter_box2d_set_scale_factor:
 * @box2d: a #ClutterBox2D
 * @scale_factor: The scaling factor
 *
 * Sets the scale factor of pixels to world units.
 */
void  clutter_box2d_set_scale_factor (ClutterBox2D *box2d,
                                      gfloat        scale_factor);

/**
 * clutter_box2d_get_scale_factor:
 * @box2d: a #ClutterBox2D
 *
 * Gets the scale factor of pixels to world units.
 *
 * Returns: The scale factor of pixels to world units.
 */
gfloat  clutter_box2d_get_scale_factor (ClutterBox2D *box2d);

/**
 * SECTION:clutter-box2d-actor
 * @short_description: Options for the children of ClutterBox2D
 *
 * The children of a ClutterBox2D container are accessed with both
 * the instance of ClutterBox2D and the child actor itself.
 */



/**
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

G_END_DECLS

#endif

#include <clutter-box2d/clutter-box2d-child.h>
#include <clutter-box2d/clutter-box2d-collision.h>
#include <clutter-box2d/clutter-box2d-joint.h>
#include <clutter-box2d/clutter-box2d-util.h>
