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
  ClutterGroup         parent_instance;
  ClutterBox2DPrivate *priv;
};

struct _ClutterBox2DClass
{
  /*< private >*/
  ClutterGroupClass parent_class;
};

typedef enum 
{
  CLUTTER_BOX2D_UNINITIALIZED = 0,
  CLUTTER_BOX2D_DYNAMIC,  /* The body is affected by collisions */
  CLUTTER_BOX2D_STATIC,   /* The body affects collisions but is immobile */
  CLUTTER_BOX2D_META      /* The body has position closely tracked to actor
                             and a body but does not affect collisions.
                           */
} ClutterBox2DType;

GType            clutter_box2d_get_type         (void) G_GNUC_CONST;
void             clutter_box2d_set_playing      (ClutterBox2D        *box2d,
                                                 gboolean             playing);
gboolean         clutter_box2d_get_playing      (ClutterBox2D        *box2d);
void             clutter_box2d_set_gravity      (ClutterBox2D        *box2d,
                                                 const ClutterVertex *gravity);
void             clutter_box2d_actor_set_type   (ClutterBox2D        *box2d,
                                                 ClutterActor        *actor,
                                                 ClutterBox2DType     type);
ClutterBox2DType clutter_box2d_actor_get_type   (ClutterBox2D        *box2d,
                                                 ClutterActor        *actor);


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

void clutter_box2d_actor_apply_force         (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              ClutterVertex    *force,
                                              ClutterVertex    *point);

void clutter_box2d_actor_apply_impulse       (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              ClutterVertex    *force,
                                              ClutterVertex    *point);

void clutter_box2d_actor_apply_torque        (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gdouble           torque);

void clutter_box2d_actor_set_bullet          (ClutterBox2D     *box2d,
                                              ClutterActor     *actor,
                                              gboolean          is_bullet);

void clutter_box2d_actor_is_bullet           (ClutterBox2D     *box2d,
                                              ClutterActor     *actor);


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

ClutterBox2DJoint *clutter_box2d_add_mouse_joint    (ClutterBox2D     *box2d,
                                                     ClutterActor     *actor,
                                                     ClutterVertex    *target);

void clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *mouse_joint,
                                              const ClutterVertex *target);

void clutter_box2d_joint_remove              (ClutterBox2DJoint   *joint);



G_END_DECLS

#endif
