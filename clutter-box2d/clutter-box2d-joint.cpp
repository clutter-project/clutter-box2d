/* clutter-box2d - Clutter box2d integration
 *
 * Copyright 2007, 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "clutter-box2d-child.h"
#include "clutter-box2d-private.h"
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
  ClutterBox2DChild     *actor1;
  ClutterBox2DChild     *actor2;
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
  ClutterBox2DPrivate *priv = box2d->priv;
  b2RevoluteJointDef jd;
  /*b2DistanceJointDef jd;*/
  /*b2PrismaticJointDef jd;*/
  b2Vec2 anchor (x_a, y_a);

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor_a), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor_b), NULL);

  /*jd.bodyA = clutter_box2d_get_child (box2d, actor_a)->body;
  jd.bodyB = clutter_box2d_get_child (box2d, actor_b)->body;*/
  /*jd.localAnchorA.Set(x_a, y_a);
  jd.localAnchorB.Set(x_b, y_b);*/
  /*jd.length = min_len;
  jd.lowerTranslation = min_len;
  jd.upperTranslation = max_len;*/
  /*jd.enableLimit = true;*/
  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_child (box2d, actor_a)->priv->body,
                clutter_box2d_get_child (box2d, actor_b)->priv->body,
                anchor);
  priv->world->CreateJoint (&jd);

  return NULL;
}

static ClutterBox2DJoint *
joint_new (ClutterBox2D *box2d,
           b2Joint      *joint)
{
  ClutterBox2DPrivate *priv = box2d->priv;
  ClutterBox2DJoint *self = g_new0 (ClutterBox2DJoint, 1);
  self->box2d = box2d;
  self->joint = joint;

  self->actor1 = (ClutterBox2DChild*)
      g_hash_table_lookup (priv->bodies, joint->GetBodyA());
  if (self->actor1)
    {
      self->actor1->priv->joints =
        g_list_append (self->actor1->priv->joints, self);
    }
  self->actor2 = (ClutterBox2DChild*)
      g_hash_table_lookup (priv->bodies, joint->GetBodyB());
  if (self->actor2)
    {
      self->actor2->priv->joints =
        g_list_append (self->actor2->priv->joints, self);
    }

  return self;
}

void
clutter_box2d_joint_destroy (ClutterBox2DJoint *joint)
{
  g_return_if_fail (joint);

  joint->box2d->priv->world->DestroyJoint (joint->joint);

  if (joint->actor1)
    {
      joint->actor1->priv->joints =
        g_list_remove (joint->actor1->priv->joints, joint);
    }
  if (joint->actor2)
    {
      joint->actor2->priv->joints =
        g_list_remove (joint->actor2->priv->joints, joint);
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
  ClutterBox2DPrivate *priv;
  b2DistanceJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  priv = box2d->priv;

  jd.collideConnected = false;
  jd.bodyA = clutter_box2d_get_child (box2d, actor1)->priv->body;
  jd.bodyB = clutter_box2d_get_child (box2d, actor2)->priv->body;
  jd.localAnchorA = b2Vec2( (anchor1->x) * priv->scale_factor,
                            (anchor1->y) * priv->scale_factor);
  jd.localAnchorB = b2Vec2( (anchor2->x) * priv->scale_factor,
                            (anchor2->y) * priv->scale_factor);
  jd.length = length * priv->scale_factor;
  jd.frequencyHz = frequency;
  jd.dampingRatio = damping_ratio;

  return joint_new (box2d, priv->world->CreateJoint (&jd));
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
  ClutterBox2DPrivate *priv;
  b2RevoluteJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  priv = box2d->priv;

  jd.collideConnected = false;
  jd.bodyA = clutter_box2d_get_child (box2d, actor1)->priv->body;
  jd.bodyB = clutter_box2d_get_child (box2d, actor2)->priv->body;
  jd.localAnchorA = b2Vec2( (anchor1->x) * priv->scale_factor,
                            (anchor1->y) * priv->scale_factor);
  jd.localAnchorB = b2Vec2( (anchor2->x) * priv->scale_factor,
                            (anchor2->y) * priv->scale_factor);
  jd.referenceAngle = reference_angle;

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_revolute_joint2 (ClutterBox2D        *box2d,
                                   ClutterActor        *actor1,
                                   ClutterActor        *actor2,
                                   const ClutterVertex *anchor)
{
  ClutterBox2DPrivate *priv;
  b2RevoluteJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor != NULL, NULL);

  priv = box2d->priv;

  b2Vec2 ancho  ( (anchor->x) * priv->scale_factor,
                  (anchor->y) * priv->scale_factor);

  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_child (box2d, actor1)->priv->body,
                clutter_box2d_get_child (box2d, actor2)->priv->body,
                ancho);
  return joint_new (box2d, priv->world->CreateJoint (&jd));
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
  ClutterBox2DPrivate *priv;
  b2PrismaticJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);

  priv = box2d->priv;

  jd.collideConnected = false;
  jd.bodyA = clutter_box2d_get_child (box2d, actor1)->priv->body;
  jd.bodyB = clutter_box2d_get_child (box2d, actor2)->priv->body;
  jd.localAnchorA = b2Vec2( (anchor1->x) * priv->scale_factor,
                            (anchor1->y) * priv->scale_factor);
  jd.localAnchorB = b2Vec2( (anchor2->x) * priv->scale_factor,
                            (anchor2->y) * priv->scale_factor);
  jd.lowerTranslation = min_length * priv->scale_factor;
  jd.upperTranslation = max_length * priv->scale_factor;
  jd.enableLimit = true;
  jd.localAxis1 = b2Vec2( (axis->x),
                          (axis->y));

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_pulley_joint (ClutterBox2D        *box2d,
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
                                gdouble              ratio)
{
  ClutterBox2DPrivate *priv;
  b2PulleyJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);
  g_return_val_if_fail (ground_anchor1 != NULL, NULL);
  g_return_val_if_fail (ground_anchor2 != NULL, NULL);

  priv = box2d->priv;

  jd.collideConnected = false;
  jd.bodyA = clutter_box2d_get_child (box2d, actor1)->priv->body;
  jd.bodyB = clutter_box2d_get_child (box2d, actor2)->priv->body;
  jd.groundAnchorA = b2Vec2 (ground_anchor1->x * priv->scale_factor,
                             ground_anchor1->y * priv->scale_factor);
  jd.groundAnchorB = b2Vec2 (ground_anchor2->x * priv->scale_factor,
                             ground_anchor2->y * priv->scale_factor);
  jd.localAnchorA = b2Vec2 (anchor1->x * priv->scale_factor,
                            anchor1->y * priv->scale_factor);
  jd.localAnchorB = b2Vec2 (anchor2->x * priv->scale_factor,
                            anchor2->y * priv->scale_factor);
  jd.ratio = ratio;
  jd.lengthA = length1 * priv->scale_factor;
  jd.lengthB = length2 * priv->scale_factor;
  jd.maxLengthA = max_length1 * priv->scale_factor;
  jd.maxLengthB = max_length2 * priv->scale_factor;

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_pulley_joint2 (ClutterBox2D        *box2d,
                                 ClutterActor        *actor1,
                                 ClutterActor        *actor2,
                                 const ClutterVertex *anchor1,
                                 const ClutterVertex *anchor2,
                                 const ClutterVertex *ground_anchor1,
                                 const ClutterVertex *ground_anchor2,
                                 gdouble              max_length1,
                                 gdouble              max_length2,
                                 gdouble              ratio)
{
  ClutterBox2DPrivate *priv;
  b2PulleyJointDef jd;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor1), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor2), NULL);
  g_return_val_if_fail (anchor1 != NULL, NULL);
  g_return_val_if_fail (anchor2 != NULL, NULL);
  g_return_val_if_fail (ground_anchor1 != NULL, NULL);
  g_return_val_if_fail (ground_anchor2 != NULL, NULL);

  priv = box2d->priv;

  jd.Initialize (clutter_box2d_get_child (box2d, actor1)->priv->body,
                 clutter_box2d_get_child (box2d, actor2)->priv->body,
                 b2Vec2 (ground_anchor1->x * priv->scale_factor,
                         ground_anchor1->y * priv->scale_factor),
                 b2Vec2 (ground_anchor2->x * priv->scale_factor,
                         ground_anchor2->y * priv->scale_factor),
                 b2Vec2 (anchor1->x * priv->scale_factor,
                         anchor1->y * priv->scale_factor),
                 b2Vec2 (anchor2->x * priv->scale_factor,
                         anchor2->y * priv->scale_factor),
                 ratio);

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *
clutter_box2d_add_mouse_joint (ClutterBox2D        *box2d,
                               ClutterActor        *actor,
                               const ClutterVertex *target)
{
  ClutterBox2DPrivate *priv;
  b2MouseJointDef md;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), NULL);
  g_return_val_if_fail (target != NULL, NULL);

  priv = box2d->priv;

  md.bodyA = priv->ground_body;
  md.bodyB = clutter_box2d_get_child (box2d, actor)->priv->body;
  md.target = b2Vec2( (target->x) * priv->scale_factor,
                      (target->y) * priv->scale_factor);
  md.bodyA->SetAwake (false);
  md.maxForce = 5100.0f * md.bodyB->GetMass ();

  return joint_new (box2d, priv->world->CreateJoint(&md));
}

void
clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *joint,
                                         const ClutterVertex *target)
{
  b2Vec2 b2target;
  ClutterBox2DPrivate *priv;

  g_return_if_fail (joint != NULL);
  g_return_if_fail (target != NULL);

  priv = joint->box2d->priv;

  b2target = b2Vec2( (target->x) * priv->scale_factor,
                     (target->y) * priv->scale_factor);

  static_cast<b2MouseJoint*>(joint->joint)->SetTarget(b2target);
}
