/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DActor class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#define SCALE_FACTOR        0.05
#define INV_SCALE_FACTOR    (1.0/SCALE_FACTOR)
#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter-child-meta.h>
#include "clutter-box2d-actor.h"
#include "clutter-box2d.h"
#include "math.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <clutter/clutter.h>

G_DEFINE_TYPE (ClutterBox2DActor, clutter_box2d_actor, CLUTTER_TYPE_CHILD_META);

#define CLUTTER_BOX2D_ACTOR_GET_PRIVATE(obj) \
      (G_TYPE_INSTANCE_GET_PRIVATE ((obj),   \
       CLUTTER_TYPE_BOX2D_ACTOR,             \
       ClutterBox2DActorPrivate))

enum
{
  PROP_0,
  PROP_IS_BULLET,
  PROP_LINEAR_VELOCITY,
  PROP_TYPE,
};

struct _ClutterBox2DActorPrivate {
    /* currently empty, since ClutterBox2D is currently tightly coupled
     * with this class, everything is accessed through the instance structure.
     */
};

static void      dispose     (GObject               *object);

ClutterBox2DType
clutter_box2d_actor_get_type (ClutterBox2DActor *box2d_actor)
{
  return box2d_actor->type;
}


void clutter_box2d_actor_set_bullet          (ClutterBox2DActor *box2d_actor,
                                              gboolean          is_bullet)
{
  box2d_actor->body->SetBullet (is_bullet);
}

gboolean
clutter_box2d_actor_is_bullet (ClutterBox2DActor *box2d_actor)
{
  return box2d_actor->body->IsBullet ();
}

ClutterBox2DType
clutter_box2d_actor_get_type2 (ClutterBox2D     *box2d,
                               ClutterActor     *actor)
{
  ClutterBox2DActor *box2d_actor;
  box2d_actor = CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta (CLUTTER_CONTAINER (box2d), actor));
  return box2d_actor->type;
}

void
clutter_box2d_actor_set_type (ClutterBox2DActor *box2d_actor,
                              ClutterBox2DType   type)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D(clutter_child_meta_get_container (CLUTTER_CHILD_META(box2d_actor)));
  b2World *world = ((b2World*)(box2d->world));

  if (box2d_actor->type == type)
    return;

  if (box2d_actor->type != CLUTTER_BOX2D_NONE)
    {
      g_assert (box2d_actor->body);

      g_hash_table_remove (box2d->bodies, box2d_actor->body);
      world->DestroyBody (box2d_actor->body);
      box2d_actor->body = NULL;
      box2d_actor->shape = NULL;
      box2d_actor->type = CLUTTER_BOX2D_NONE;
    }

  if (type == CLUTTER_BOX2D_DYNAMIC ||
      type == CLUTTER_BOX2D_STATIC)
    {
      b2BodyDef bodyDef;

      bodyDef.linearDamping = 0.0f;
      bodyDef.angularDamping = 0.02f;

      SYNCLOG ("making an actor to be %s\n",
               type == CLUTTER_BOX2D_STATIC ? "static" : "dynamic");

      box2d_actor->type = type;

      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          box2d_actor->body = world->CreateDynamicBody (&bodyDef);
        }
      else if (type == CLUTTER_BOX2D_STATIC)
        {
          box2d_actor->body = world->CreateStaticBody (&bodyDef);
        }
      sync_body (box2d_actor);
      box2d_actor->body->SetMassFromShapes ();
    }
  g_hash_table_insert (box2d->bodies, box2d_actor->body, box2d_actor);
}


static void
clutter_box2d_actor_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  ClutterChildMeta  *child_meta = CLUTTER_CHILD_META (gobject);
  ClutterBox2DActor *box2d_actor = CLUTTER_BOX2D_ACTOR (gobject);
 
  switch (prop_id)
    {
    case PROP_IS_BULLET:
      clutter_box2d_actor_set_bullet (box2d_actor,
                                      g_value_get_boolean (value));
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex *vertex = (ClutterVertex*)g_value_get_boxed (value);
        b2Vec2 b2velocity (CLUTTER_UNITS_TO_FLOAT (vertex->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (vertex->y) * SCALE_FACTOR);
        box2d_actor->body->SetLinearVelocity (b2velocity);
      }
      break;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_actor_get_property (GObject      *gobject,
                                  guint         prop_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  ClutterChildMeta  *child_meta = CLUTTER_CHILD_META (gobject);
  ClutterBox2DActor *box2d_actor = CLUTTER_BOX2D_ACTOR (gobject);
 
  switch (prop_id)
    {
    case PROP_IS_BULLET:
      g_value_set_boolean (value, clutter_box2d_actor_is_bullet (box2d_actor));
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex *vertex = (ClutterVertex*)g_value_get_boxed (value);
        b2Vec2 b2velocity (CLUTTER_UNITS_TO_FLOAT (vertex->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (vertex->y) * SCALE_FACTOR);
        box2d_actor->body->SetLinearVelocity (b2velocity);
      }
      break;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_actor_class_init (ClutterBox2DActorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose      = dispose;
  gobject_class->set_property = clutter_box2d_actor_set_property;
  gobject_class->get_property = clutter_box2d_actor_get_property;

  g_object_class_install_property (gobject_class,
                                   PROP_LINEAR_VELOCITY,
                                   g_param_spec_boxed ("linear-velocity",
                                                       "Linear velocity",
                                                       "Linear velocity",
                                                       CLUTTER_TYPE_VERTEX,
                                                       G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_IS_BULLET,
                                   g_param_spec_boolean ("is-bullet",
                                                         "Is bullet",
                                                         "Wheter this object is a bullet (fast moving object that should not be allowed tunneling through other dynamic objects.)",
                                                         FALSE,
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DActorPrivate));
}

static void
clutter_box2d_actor_init (ClutterBox2DActor *self)
{
  self->priv = CLUTTER_BOX2D_ACTOR_GET_PRIVATE (self);
}

static void
dispose (GObject *object)
{
  ClutterBox2DActor *self = CLUTTER_BOX2D_ACTOR (object);

  while (self->joints)
    {
      clutter_box2d_joint_destroy ((ClutterBox2DJoint*)self->joints->data);
    }

  G_OBJECT_CLASS (clutter_box2d_actor_parent_class)->dispose (object);
}
