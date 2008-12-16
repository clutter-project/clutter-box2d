/* clutter-box2d - Clutter box2d integration
 *
 * Copyright 2007, 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#define SCALE_FACTOR        0.05
#define INV_SCALE_FACTOR    (1.0/SCALE_FACTOR)

#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "clutter-box2d-actor.h"
#include "math.h"

typedef enum 
{
  CLUTTER_BOX2D_JOINT_DEAD,      /* An associated actor has been killed off */
  CLUTTER_BOX2D_JOINT_DISTANCE,
  CLUTTER_BOX2D_JOINT_PRISMATIC,
  CLUTTER_BOX2D_JOINT_REVOLUTE,
  CLUTTER_BOX2D_JOINT_MOUSE
} ClutterBox2DJointType;

/* A Box2DJoint contains all relevant tracking information
 * for a box2d joint.
 */
struct _ClutterBox2DJoint
{
  ClutterBox2D     *box2d;/* The ClutterBox2D this joint management struct
                             belongs to */
  ClutterBox2DJointType *type;  /* The type of joint */

  b2Joint               *joint; /* Box2d joint*/

  /* The actors hooked up to this joint, for a JOINT_MOUSE, only actor1 will
   * be set actor2 will be NULL
   */
  ClutterBox2DActor     *actor1;  
  ClutterBox2DActor     *actor2; 
};


ClutterBox2DJoint *
clutter_box2d_add_joint (ClutterBox2D     *box2d,
                         ClutterActor     *actor_a,
                         ClutterActor     *actor_b,
                         gdouble           x_a,
                         gdouble           y_a,
                         gdouble           x_b,
                         gdouble           y_b,
                         gdouble           min_len,
                         gdouble           max_len)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2RevoluteJointDef jd;
  /*b2DistanceJointDef jd;*/
  /*b2PrismaticJointDef jd;*/
  b2Vec2 anchor (x_a, y_a);

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor_a), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor_b), NULL);

  /*jd.body1 = clutter_box2d_get_actor (box2d, actor_a)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor_b)->body;*/
  /*jd.localAnchor1.Set(x_a, y_a);
  jd.localAnchor2.Set(x_b, y_b);*/
  /*jd.length = min_len;
  jd.lowerTranslation = min_len;
  jd.upperTranslation = max_len;*/
  /*jd.enableLimit = true;*/
  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_actor (box2d, actor_a)->body,
                clutter_box2d_get_actor (box2d, actor_b)->body,
                anchor);
  ((b2World*)(box2d->world))->CreateJoint (&jd);

  return NULL;
}

static ClutterBox2DJoint *
joint_new (ClutterBox2D *box2d,
           b2Joint      *joint)
{
   /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
   ClutterBox2DJoint *self = g_new0 (ClutterBox2DJoint, 1);
   self->box2d = box2d;
   self->joint = joint;

   self->actor1 = (ClutterBox2DActor*)
        g_hash_table_lookup (box2d->bodies, joint->GetBody1());
   if (self->actor1)
     {
        self->actor1->joints = g_list_append (self->actor1->joints, self);
     }
   self->actor2 = (ClutterBox2DActor*)
        g_hash_table_lookup (box2d->bodies, joint->GetBody2());
   if (self->actor2)
     {
        self->actor2->joints = g_list_append (self->actor2->joints, self);
     }

   return self;
}

void
clutter_box2d_joint_destroy (ClutterBox2DJoint *joint)
{
  g_return_if_fail (joint);


  ((b2World*)joint->box2d->world)->DestroyJoint (joint->joint);

   if (joint->actor1)
     {
        joint->actor1->joints = g_list_remove (joint->actor1->joints, joint);
     }
   if (joint->actor2)
     {
        joint->actor2->joints = g_list_remove (joint->actor2->joints, joint);
     }

  g_free (joint);
}


ClutterBox2DJoint *
clutter_box2d_add_distance_joint (ClutterBox2D        *box2d,
                                  ClutterActor        *actor1,
                                  ClutterActor        *actor2,
                                  const ClutterVertex *anchor1,
                                  const ClutterVertex *anchor2,
                                  gdouble              length,
                                  gdouble              frequency,
                                  gdouble              damping_ratio)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2DistanceJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y) * SCALE_FACTOR);
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y) * SCALE_FACTOR);
  jd.length = length * SCALE_FACTOR;
  jd.frequencyHz = frequency;
  jd.dampingRatio = damping_ratio;

  return joint_new (box2d, ((b2World*)box2d->world)->CreateJoint (&jd));
}


ClutterBox2DJoint *
clutter_box2d_add_distance_joint2 (ClutterBox2D        *box2d,
                                   ClutterActor        *actor1,
                                   ClutterActor        *actor2,
                                   const ClutterVertex *anchor1,
                                   const ClutterVertex *anchor2,
                                   gdouble              frequency,
                                   gdouble              damping_ratio)
{
  g_warning ("clutter_box2d_add_distance_joint2 not yet implemented");
  /* this one should compute the length automatically based on the
   * initial configuration?
   */
  return NULL;
}


ClutterBox2DJoint *
clutter_box2d_add_revolute_joint (ClutterBox2D        *box2d,
                                  ClutterActor        *actor1,
                                  ClutterActor        *actor2,
                                  const ClutterVertex *anchor1,
                                  const ClutterVertex *anchor2,
                                  gdouble              reference_angle)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2RevoluteJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y) * SCALE_FACTOR);
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y) * SCALE_FACTOR);
  jd.referenceAngle = reference_angle;

  return joint_new (box2d, ((b2World*)box2d->world)->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_revolute_joint2 (ClutterBox2D        *box2d,
                                   ClutterActor        *actor1,
                                   ClutterActor        *actor2,
                                   const ClutterVertex *anchor)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2RevoluteJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor != NULL, NULL);

  b2Vec2 ancho  (CLUTTER_UNITS_TO_FLOAT (anchor->x) * SCALE_FACTOR,
                 CLUTTER_UNITS_TO_FLOAT (anchor->y) * SCALE_FACTOR);

  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_actor (box2d, actor1)->body,
                clutter_box2d_get_actor (box2d, actor2)->body,
                ancho);
  return joint_new (box2d, ((b2World*)box2d->world)->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_prismatic_joint (ClutterBox2D        *box2d,
                                   ClutterActor        *actor1,
                                   ClutterActor        *actor2,
                                   const ClutterVertex *anchor1,
                                   const ClutterVertex *anchor2,
                                   gdouble              min_length,
                                   gdouble              max_length,
                                   const ClutterVertex *axis)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2PrismaticJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y) * SCALE_FACTOR);
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y) * SCALE_FACTOR);
  jd.lowerTranslation = min_length * SCALE_FACTOR;
  jd.lowerTranslation = max_length * SCALE_FACTOR;
  jd.enableLimit = true;
  jd.localAxis1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (axis->x),
                         CLUTTER_UNITS_TO_FLOAT (axis->y));

  return joint_new (box2d, ((b2World*)box2d->world)->CreateJoint (&jd));
}


ClutterBox2DJoint *
clutter_box2d_add_mouse_joint (ClutterBox2D        *box2d,
                               ClutterActor        *actor,
                               const ClutterVertex *target)
{
  /*ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);*/
  b2MouseJointDef md;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), NULL);
  g_return_val_if_fail (target != NULL, NULL);

  md.body1 = ((b2World*)box2d->world)->GetGroundBody();
  md.body2 = clutter_box2d_get_actor (box2d, actor)->body;
  md.target = b2Vec2(CLUTTER_UNITS_TO_FLOAT (target->x) * SCALE_FACTOR,
                     CLUTTER_UNITS_TO_FLOAT (target->y) * SCALE_FACTOR);
  md.body1->WakeUp ();
  md.maxForce = 5100.0f * md.body2->GetMass ();

  return joint_new (box2d, ((b2World*)box2d->world)->CreateJoint(&md));
}

void
clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *joint,
                                         const ClutterVertex *target)
{
  b2Vec2 b2target;

  g_return_if_fail (joint != NULL);
  g_return_if_fail (target != NULL);
 
  b2target = b2Vec2(CLUTTER_UNITS_TO_FLOAT (target->x) * SCALE_FACTOR,
                    CLUTTER_UNITS_TO_FLOAT (target->y) * SCALE_FACTOR);

  static_cast<b2MouseJoint*>(joint->joint)->SetTarget(b2target);
}
