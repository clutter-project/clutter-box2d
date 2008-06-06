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

/* defined in clutter-box2d-actor.h */
void _clutter_box2d_sync_body (ClutterBox2DActor *box2d_actor);

G_BEGIN_DECLS
void tidy_cursor (gint x, gint y);
G_END_DECLS

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
  PROP_ANGULAR_VELOCITY,
  PROP_MODE,
  PROP_MANIPULATABLE,
};

struct _ClutterBox2DActorPrivate {
  gboolean manipulatable;
  guint    press_handler;
  guint    release_handler;
  guint    motion_handler;
  gboolean was_reactive;

  ClutterBox2DJoint *mouse_joint;
  ClutterUnit        start_x, start_y;
};

static void     dispose                     (GObject      *object);
static gboolean clutter_box2d_actor_press   (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);
static gboolean clutter_box2d_actor_release (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);
static gboolean clutter_box2d_actor_motion  (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);


ClutterBox2DActor *
clutter_box2d_get_actor (ClutterBox2D   *box2d,
                          ClutterActor   *actor)
{
  return CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta (CLUTTER_CONTAINER (box2d), actor));
}


static gboolean
clutter_box2d_actor_is_bullet (ClutterBox2DActor *box2d_actor)
{
  return box2d_actor->body->IsBullet ();
}

static void
clutter_box2d_actor_set_type2 (ClutterBox2DActor *box2d_actor,
                               ClutterBox2DType   type)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D(clutter_child_meta_get_container (CLUTTER_CHILD_META(box2d_actor)));
  b2World *world = ((b2World*)(box2d->world));

  if (box2d_actor->type == type)
    return;
  g_assert (!(type == 0 && box2d_actor->type != 0));

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
      _clutter_box2d_sync_body (box2d_actor);
      box2d_actor->body->SetMassFromShapes ();
    }
  g_hash_table_insert (box2d->bodies, box2d_actor->body, box2d_actor);
}

/* Set the type of physical object an actor in a Box2D group is of.
 */
static void
clutter_box2d_actor_set_type (ClutterBox2D      *box2d,
                              ClutterActor      *actor,
                              ClutterBox2DType   type)
{
  ClutterBox2DActor   *box2d_actor = CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta (
     CLUTTER_CONTAINER (box2d), actor));
  clutter_box2d_actor_set_type2 (box2d_actor, type);
}

static void
clutter_box2d_actor_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;
  ClutterBox2DActorPrivate *priv;

  child_meta = CLUTTER_CHILD_META (gobject);
  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  priv = box2d_actor->priv;
 
  switch (prop_id)
    {
    case PROP_MANIPULATABLE:
      if (g_value_get_boolean (value))
        {
          ClutterActor *actor = child_meta->actor;
          priv->manipulatable = TRUE;
          priv->was_reactive = clutter_actor_get_reactive (actor);
          clutter_actor_set_reactive (actor, TRUE);
          priv->press_handler = 
          g_signal_connect (actor, "button-press-event",
                            G_CALLBACK (clutter_box2d_actor_press),
                            child_meta);
          priv->motion_handler = 
          g_signal_connect (actor, "motion-event",
                            G_CALLBACK (clutter_box2d_actor_motion),
                            child_meta);
          priv->release_handler = 
          g_signal_connect (actor, "button-release-event",
                            G_CALLBACK (clutter_box2d_actor_release),
                            child_meta);
        }
      else
        {
          if (priv->manipulatable)
            {
              ClutterActor *actor = child_meta->actor;

              if (!priv->was_reactive)
                clutter_actor_set_reactive (actor, FALSE);
              g_signal_handler_disconnect (actor, priv->press_handler);
              g_signal_handler_disconnect (actor, priv->motion_handler);
              g_signal_handler_disconnect (actor, priv->release_handler);
              priv->manipulatable = FALSE;
            }
        }
      break;

    case PROP_IS_BULLET:
      box2d_actor->body->SetBullet (g_value_get_boolean (value));
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex *vertex = (ClutterVertex*)g_value_get_boxed (value);
        b2Vec2 b2velocity (CLUTTER_UNITS_TO_FLOAT (vertex->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (vertex->y) * SCALE_FACTOR);
        box2d_actor->body->SetLinearVelocity (b2velocity);
      }
      break;
    case PROP_ANGULAR_VELOCITY:
      box2d_actor->body->SetAngularVelocity (g_value_get_double (value));
      break;
    case PROP_MODE:
      clutter_box2d_actor_set_type2 (box2d_actor, (ClutterBox2DType)g_value_get_int (value));

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
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;
  ClutterBox2DActorPrivate *priv;

  child_meta = CLUTTER_CHILD_META (gobject);
  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  priv = box2d_actor->priv;
 

 
  switch (prop_id)
    {
    case PROP_IS_BULLET:
      g_value_set_boolean (value, box2d_actor->body->IsBullet ());
      break;
    case PROP_LINEAR_VELOCITY:
      {
        /* FIXME: this is setting it, not getting it! */
        ClutterVertex *vertex = (ClutterVertex*)g_value_get_boxed (value);
        b2Vec2 b2velocity (CLUTTER_UNITS_TO_FLOAT (vertex->x) * SCALE_FACTOR,
                           CLUTTER_UNITS_TO_FLOAT (vertex->y) * SCALE_FACTOR);
        box2d_actor->body->SetLinearVelocity (b2velocity);
      }
      break;
    case PROP_ANGULAR_VELOCITY:
      g_value_set_double (value, box2d_actor->body->GetAngularVelocity());
      break;
    case PROP_MODE:
      g_value_set_int (value, box2d_actor->type);
      break;
    case PROP_MANIPULATABLE:
      g_value_set_boolean (value, priv->manipulatable);
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
                                   PROP_ANGULAR_VELOCITY,
                                   g_param_spec_double ("angular-velocity",
                                                       "Angular velocity",
                                                       "Angular velocity",
                                                       -5000.0, 5000.0, 0.0,
                                                       (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_MODE,
                                   g_param_spec_int ("mode",
                                                     "Box2d Mode",
                                   "The mode of the actor (none, static or dynamic)",
                                                     0, G_MAXINT, 0,
                                                     (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_IS_BULLET,
                                   g_param_spec_boolean ("is-bullet",
                                                         "Is bullet",
                                     "Whether this object is a bullet (fast moving "
                                     "object that should not be allowed tunneling "
                                     "through other dynamic objects.)",
                                                         FALSE,
                                                         (GParamFlags)G_PARAM_READWRITE));


  g_object_class_install_property (gobject_class,
                                   PROP_MANIPULATABLE,
                                   g_param_spec_boolean ("manipulatable",
                                                         "Manipulatable",
                                     "Whether the user is able to interact (using a pointer device) with this actor or not.)",
                                                         FALSE,
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DActorPrivate));
}

static void
clutter_box2d_actor_init (ClutterBox2DActor *self)
{
  self->priv = CLUTTER_BOX2D_ACTOR_GET_PRIVATE (self);
  self->priv->manipulatable = FALSE;
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


static gboolean
clutter_box2d_actor_press (ClutterActor *actor,
                           ClutterEvent *event,
                           gpointer      data)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;
  ClutterBox2DActorPrivate *priv;

  child_meta = CLUTTER_CHILD_META (data);
  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  priv = box2d_actor->priv;

  if (clutter_box2d_get_simulating (
      CLUTTER_BOX2D (clutter_actor_get_parent (actor))))
    {

      priv->start_x = CLUTTER_UNITS_FROM_INT (event->button.x);
      priv->start_y = CLUTTER_UNITS_FROM_INT (event->button.y);

      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (actor),
        priv->start_x, priv->start_y,
        &priv->start_x, &priv->start_y);

      g_object_ref (actor);
#if 0
      clutter_grab_pointer_device (actor, device);
#else
      clutter_grab_pointer (actor);
#endif

      priv->mouse_joint = clutter_box2d_add_mouse_joint (CLUTTER_BOX2D (
                        clutter_actor_get_parent (actor)),
                        actor, &(ClutterVertex){priv->start_x, priv->start_y});
    }
  return FALSE;
}

static gboolean
clutter_box2d_actor_motion (ClutterActor *actor,
                            ClutterEvent *event,
                            gpointer      data)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;
  ClutterBox2DActorPrivate *priv;

  child_meta = CLUTTER_CHILD_META (data);
  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  priv = box2d_actor->priv;

  if (priv->mouse_joint)
    {
      ClutterUnit x;
      ClutterUnit y;
      ClutterUnit dx;
      ClutterUnit dy;

      x = CLUTTER_UNITS_FROM_INT (event->motion.x);
      y = CLUTTER_UNITS_FROM_INT (event->motion.y);

      /* something like this is needed since the pointer is grabbed */
      /* tidy_cursor (event->motion.x, event->motion.y); */

      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (actor),
        x, y,
        &x, &y);

       dx = x - priv->start_x;
       dy = y - priv->start_y;

       ClutterVertex target = { x, y };
       clutter_box2d_mouse_joint_update_target (priv->mouse_joint, &target);
    }
  return FALSE;
}


static gboolean
clutter_box2d_actor_release (ClutterActor *actor,
                             ClutterEvent *event,
                             gpointer      data)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;
  ClutterBox2DActorPrivate *priv;

  child_meta = CLUTTER_CHILD_META (data);
  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  priv = box2d_actor->priv;

  if (priv->mouse_joint)
    {
      clutter_box2d_joint_destroy (priv->mouse_joint);
      priv->mouse_joint = NULL;

      /* perhaps a new special call is needed here for multi-touch? */
      clutter_ungrab_pointer ();

      g_object_unref (actor);

      /* since the ungrab also was valid for this release we redeliver the
       * event to maintain the state of the click count.
       */
      if(1){ 
        ClutterEvent *synthetic_release;
        synthetic_release = clutter_event_new (CLUTTER_BUTTON_RELEASE);
        memcpy (synthetic_release, event, sizeof (ClutterButtonEvent));
        synthetic_release->any.source = NULL;
        clutter_do_event (synthetic_release); /* skip queue */
        clutter_event_free (synthetic_release);
      }

      return FALSE;
    }

  return FALSE;
}

