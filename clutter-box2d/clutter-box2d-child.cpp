/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DChild class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical
 * state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Copyright 2010 Intel Corporation
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#define SCALE_FACTOR        0.05
#define INV_SCALE_FACTOR    (1.0/SCALE_FACTOR)
#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d-child.h"
#include "clutter-box2d.h"
#include "math.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "clutter-box2d-marshal.h"

/* defined in clutter-box2d.cpp */
void _clutter_box2d_sync_body (ClutterBox2DChild *box2d_child);

G_DEFINE_TYPE (ClutterBox2DChild, clutter_box2d_child, CLUTTER_TYPE_CHILD_META);

#define CLUTTER_BOX2D_CHILD_GET_PRIVATE(obj) \
      (G_TYPE_INSTANCE_GET_PRIVATE ((obj),   \
       CLUTTER_TYPE_BOX2D_CHILD,             \
       ClutterBox2DChildPrivate))

enum
{
  PROP_0,
  PROP_IS_BULLET,
  PROP_IS_CIRCLE,
  PROP_OUTLINE,
  PROP_DENSITY,
  PROP_FRICTION,
  PROP_RESTITUTION,
  PROP_LINEAR_VELOCITY,
  PROP_ANGULAR_VELOCITY,
  PROP_MODE,
  PROP_MANIPULATABLE,
};

enum
{
  COLLISION,
  LAST_SIGNAL
};

static gint box2d_child_signals[LAST_SIGNAL];

struct _ClutterBox2DChildPrivate {
  gboolean manipulatable;
  guint    press_handler;
  guint    captured_handler;
  gboolean was_reactive;

  gint               device_id;
  ClutterBox2DJoint *mouse_joint;
  gfloat        start_x, start_y;
};

static void     dispose                     (GObject      *object);
static void     clutter_box2d_child_constructed (GObject *object);
static gboolean clutter_box2d_child_press   (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);
static gboolean clutter_box2d_child_captured_event (ClutterActor *stage,
                                                    ClutterEvent *event,
                                                    gpointer      data);



static gboolean
clutter_box2d_child_is_bullet (ClutterBox2DChild *box2d_child)
{
  return box2d_child->body->IsBullet ();
}

static void
clutter_box2d_child_set_type2 (ClutterBox2DChild *box2d_child,
                               ClutterBox2DType   type)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D(clutter_child_meta_get_container (CLUTTER_CHILD_META(box2d_child)));
  b2World *world = ((b2World*)(box2d->world));

  if (box2d_child->type == type)
    return;

  if (box2d_child->type != CLUTTER_BOX2D_NONE)
    {
      g_assert (box2d_child->body);

      g_hash_table_remove (box2d->bodies, box2d_child->body);
      world->DestroyBody (box2d_child->body);
      box2d_child->body = NULL;
      box2d_child->shape = NULL;
      box2d_child->type = CLUTTER_BOX2D_NONE;
    }

  if (type == CLUTTER_BOX2D_DYNAMIC ||
      type == CLUTTER_BOX2D_STATIC)
    {
      b2BodyDef bodyDef;

      bodyDef.linearDamping = 0.5f;
      bodyDef.angularDamping = 0.5f;


      SYNCLOG ("making an actor to be %s\n",
               type == CLUTTER_BOX2D_STATIC ? "static" : "dynamic");

      box2d_child->type = type;

      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          box2d_child->body = world->CreateBody (&bodyDef);
        }
      else if (type == CLUTTER_BOX2D_STATIC)
        {

          box2d_child->body = world->CreateBody (&bodyDef);
        }
      _clutter_box2d_sync_body (box2d_child);


      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          box2d_child->body->SetMassFromShapes ();
        }

      g_hash_table_insert (box2d->bodies, box2d_child->body, box2d_child);
    }
}

/* Set the type of physical object an actor in a Box2D group is of.
 */
static void
clutter_box2d_child_set_type (ClutterBox2D      *box2d,
                              ClutterActor      *actor,
                              ClutterBox2DType   type)
{
  ClutterBox2DChild *box2d_child;
  
  box2d_child =
    CLUTTER_BOX2D_CHILD (clutter_container_get_child_meta (CLUTTER_CONTAINER (box2d), actor));
  clutter_box2d_child_set_type2 (box2d_child, type);
}

static inline void
clutter_box2d_child_refresh_shape (ClutterBox2DChild *box2d_child)
{
  if (box2d_child->shape)
    {
      box2d_child->body->DestroyShape (box2d_child->shape);
      box2d_child->shape = NULL;
      _clutter_box2d_sync_body (box2d_child);
      if (box2d_child->type == CLUTTER_BOX2D_DYNAMIC)
        box2d_child->body->SetMassFromShapes ();
    }
}

static void
clutter_box2d_child_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DChild *box2d_child;
  ClutterBox2DChildPrivate *priv;

  child_meta = CLUTTER_CHILD_META (gobject);
  box2d_child = CLUTTER_BOX2D_CHILD (child_meta);
  priv = box2d_child->priv;
 
  switch (prop_id)
    {
    case PROP_MANIPULATABLE:
      if (g_value_get_boolean (value))
        {
          if (!priv->manipulatable)
            {
              ClutterActor *actor = child_meta->actor;
              priv->manipulatable = TRUE;
              priv->was_reactive = clutter_actor_get_reactive (actor);
              clutter_actor_set_reactive (actor, TRUE);
              priv->press_handler = 
              g_signal_connect (actor, "button-press-event",
                                G_CALLBACK (clutter_box2d_child_press),
                                child_meta);

              g_object_notify (gobject, "manipulatable");
            }
        }
      else
        {
          if (priv->manipulatable)
            {
              ClutterActor *actor = child_meta->actor;

              if (!priv->was_reactive)
                clutter_actor_set_reactive (actor, FALSE);

              g_signal_handler_disconnect (actor, priv->press_handler);
              priv->press_handler = 0;

              if (priv->captured_handler)
                {
                  g_signal_handler_disconnect (clutter_actor_get_stage (actor),
                                               priv->captured_handler);
                  priv->captured_handler = 0;
                }

              priv->manipulatable = FALSE;

              g_object_notify (gobject, "manipulatable");
            }
        }
      break;

    case PROP_IS_BULLET:
      box2d_child->body->SetBullet (g_value_get_boolean (value));
      break;
    case PROP_IS_CIRCLE:
      box2d_child->is_circle = g_value_get_boolean (value);
      clutter_box2d_child_refresh_shape (box2d_child);
      break;
    case PROP_DENSITY:
      box2d_child->density = g_value_get_float (value);
      clutter_box2d_child_refresh_shape (box2d_child);
      break;
    case PROP_FRICTION:
      box2d_child->friction = g_value_get_float (value);
      clutter_box2d_child_refresh_shape (box2d_child);
      break;
    case PROP_RESTITUTION:
      box2d_child->restitution = g_value_get_float (value);
      clutter_box2d_child_refresh_shape (box2d_child);
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex *vertex = (ClutterVertex*)g_value_get_boxed (value);
        b2Vec2 b2velocity ( (vertex->x) * SCALE_FACTOR,
                            (vertex->y) * SCALE_FACTOR);
        box2d_child->body->SetLinearVelocity (b2velocity);
      }
      break;
    case PROP_ANGULAR_VELOCITY:
      box2d_child->body->SetAngularVelocity (g_value_get_double (value));
      break;
    case PROP_MODE:
      clutter_box2d_child_set_type2 (box2d_child, (ClutterBox2DType)g_value_get_int (value));

      break;

    case PROP_OUTLINE:
      {
        GValueArray *array;

        g_object_freeze_notify (gobject);

        box2d_child->shape = NULL;

        if (box2d_child->is_circle)
          {
            box2d_child->is_circle = FALSE;
            g_object_notify (gobject, "is-circle");
          }

        g_free (box2d_child->outline);
        box2d_child->outline = NULL;

        array = (GValueArray *)g_value_get_boxed (value);
        if (array && (array->n_values > 2))
          {
            gint i;

            box2d_child->outline = g_new0 (ClutterVertex, array->n_values + 1);
            for (i = 0; i < array->n_values; i++)
              box2d_child->outline[i] = *((ClutterVertex *)
                                          g_value_get_boxed (
                                            g_value_array_get_nth (array, i)));

            /* Close the path */
            box2d_child->outline[i] = box2d_child->outline[0];
          }

        g_object_notify (gobject, "outline");

        /* Synchronise box2d state */
        clutter_box2d_child_refresh_shape (box2d_child);

        g_object_thaw_notify (gobject);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_child_get_property (GObject      *gobject,
                                  guint         prop_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DChild *box2d_child;
  ClutterBox2DChildPrivate *priv;

  child_meta = CLUTTER_CHILD_META (gobject);
  box2d_child = CLUTTER_BOX2D_CHILD (child_meta);
  priv = box2d_child->priv;
 

 
  switch (prop_id)
    {
    case PROP_IS_BULLET:
        g_value_set_boolean (value, box2d_child->body?
                                    box2d_child->body->IsBullet ():FALSE);
      break;
    case PROP_IS_CIRCLE:
      g_value_set_boolean (value, box2d_child->is_circle);
      break;
    case PROP_DENSITY:
      g_value_set_float (value, box2d_child->density);
      break;
    case PROP_FRICTION:
      g_value_set_float (value, box2d_child->friction);
      break;
    case PROP_RESTITUTION:
      g_value_set_float (value, box2d_child->restitution);
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex vertex;

        b2Vec2 velocity = box2d_child->body->GetLinearVelocity();
        vertex.x = velocity.x / SCALE_FACTOR;
        vertex.y = velocity.y / SCALE_FACTOR;
        vertex.z = 0;

        g_value_set_boxed (value, &vertex);
      }
      break;
    case PROP_ANGULAR_VELOCITY:
      g_value_set_double (value, box2d_child->body?
                          box2d_child->body->GetAngularVelocity():0.0);
      break;
    case PROP_MODE:
      g_value_set_int (value, box2d_child->type);
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
clutter_box2d_child_class_init (ClutterBox2DChildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose      = dispose;
  gobject_class->set_property = clutter_box2d_child_set_property;
  gobject_class->get_property = clutter_box2d_child_get_property;
  gobject_class->constructed  = clutter_box2d_child_constructed;

  box2d_child_signals[COLLISION] = g_signal_new ("collision",
                                 G_TYPE_FROM_CLASS (gobject_class),
                                 G_SIGNAL_RUN_LAST,
                                 0,
                                 NULL, NULL,
                                 clutter_box2d_marshal_VOID__OBJECT,
                                 G_TYPE_NONE, 1, 
                                 CLUTTER_TYPE_BOX2D_COLLISION);

  g_object_class_install_property (gobject_class,
                                   PROP_DENSITY,
                                   g_param_spec_float ("density",
                                                       "Density",
                                                       "Density",
                                                       0.f, G_MAXFLOAT, 7.f,
                                                       (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_FRICTION,
                                   g_param_spec_float ("friction",
                                                       "Friction",
                                                       "Friction",
                                                       0.f, 1.f, 0.4f,
                                                       (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_RESTITUTION,
                                   g_param_spec_float ("restitution",
                                                       "Restitution",
                                                       "Restitution",
                                                       0.f, 1.f, 0.0f,
                                                       (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_LINEAR_VELOCITY,
                                   g_param_spec_boxed ("linear-velocity",
                                                       "Linear velocity",
                                                       "Linear velocity",
                                                       CLUTTER_TYPE_VERTEX,
                                                       (GParamFlags)G_PARAM_READWRITE));


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
                                   PROP_IS_CIRCLE,
                                   g_param_spec_boolean ("is-circle",
                                                         "Is circle",
                                     "Whether this object is a circle instead of "
                                     "a rectangle.",
                                                         FALSE,
                                                         (GParamFlags)G_PARAM_READWRITE));


  g_object_class_install_property (gobject_class,
                                   PROP_MANIPULATABLE,
                                   g_param_spec_boolean ("manipulatable",
                                                         "Manipulatable",
                                     "Whether the user is able to interact (using a pointer device) with this actor or not.)",
                                                         FALSE,
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_OUTLINE,
                                   g_param_spec_value_array ("outline",
                                                             "Outline",
                                                             "ClutterVertex array describing the outline of the shape.",
                                                             g_param_spec_boxed ("vertex",
                                                                                 "Vertex",
                                                                                 "A ClutterVertex.",
                                                                                 CLUTTER_TYPE_VERTEX,
                                                                                 (GParamFlags)G_PARAM_READWRITE),
                                                             (GParamFlags)G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DChildPrivate));
}

static void
clutter_box2d_child_constructed (GObject *object)
{
  ClutterActor *actor = CLUTTER_CHILD_META (object)->actor;

  g_signal_connect_swapped (actor, "notify::natural-width",
                            G_CALLBACK (clutter_box2d_child_refresh_shape),
                            object);
  g_signal_connect_swapped (actor, "notify::natural-height",
                            G_CALLBACK (clutter_box2d_child_refresh_shape),
                            object);
}

static void
clutter_box2d_child_init (ClutterBox2DChild *self)
{
  self->priv = CLUTTER_BOX2D_CHILD_GET_PRIVATE (self);
  self->priv->manipulatable = FALSE;
  self->density = 7.0f;
  self->friction = 0.4f;
  self->restitution = 0.f;
}

static void
dispose (GObject *object)
{
  ClutterChildMeta *child_meta = CLUTTER_CHILD_META (object);
  ClutterBox2DChild *self = CLUTTER_BOX2D_CHILD (object);
  ClutterBox2DChildPrivate *priv = self->priv;

  if (child_meta->actor)
    g_signal_handlers_disconnect_by_func (child_meta->actor,
                                          (gpointer)clutter_box2d_child_refresh_shape,
                                          object);

  if (priv->captured_handler)
    {
      ClutterActor *stage = clutter_actor_get_stage (CLUTTER_ACTOR (self));
      if (stage)
        g_signal_handler_disconnect (stage, priv->captured_handler);
      priv->captured_handler = 0;
    }

  while (self->joints)
    {
      clutter_box2d_joint_destroy ((ClutterBox2DJoint*)self->joints->data);
    }

  G_OBJECT_CLASS (clutter_box2d_child_parent_class)->dispose (object);
}

static gboolean
clutter_box2d_child_press (ClutterActor *actor,
                           ClutterEvent *event,
                           gpointer      data)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DChild *box2d_child;
  ClutterBox2DChildPrivate *priv;

  child_meta = CLUTTER_CHILD_META (data);
  box2d_child = CLUTTER_BOX2D_CHILD (child_meta);
  priv = box2d_child->priv;

  if (event->button.button == 1 &&
      clutter_box2d_get_simulating (
      CLUTTER_BOX2D (clutter_actor_get_parent (actor))))
    {

      priv->start_x =  (event->button.x);
      priv->start_y =  (event->button.y);

      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (actor),
        priv->start_x, priv->start_y,
        &priv->start_x, &priv->start_y);

      priv->captured_handler =
        g_signal_connect (clutter_actor_get_stage (actor),
                          "captured-event",
                          G_CALLBACK (clutter_box2d_child_captured_event),
                          child_meta);
      g_print ("grab: %p:%i\n", actor, clutter_event_get_device_id (event));

      if (priv->mouse_joint == 0)
        {
          ClutterVertex vertex = {priv->start_x, priv->start_y};
          priv->mouse_joint = clutter_box2d_add_mouse_joint (CLUTTER_BOX2D (
                              clutter_actor_get_parent (actor)),
                              actor, &vertex);
        }

      priv->device_id = clutter_event_get_device_id (event);

      return TRUE;
    }

  return FALSE;
}

static gboolean
clutter_box2d_child_captured_event (ClutterActor *stage,
                                    ClutterEvent *event,
                                    gpointer      data)
{
  ClutterChildMeta  *child_meta;
  ClutterBox2DChild *box2d_child;
  ClutterBox2DChildPrivate *priv;

  child_meta = CLUTTER_CHILD_META (data);
  box2d_child = CLUTTER_BOX2D_CHILD (child_meta);
  priv = box2d_child->priv;
  gint id = clutter_event_get_device_id (event);

  if (id != priv->device_id)
    return FALSE;

  switch (event->type)
    {
    case CLUTTER_MOTION:
      if (priv->mouse_joint)
        {
          gfloat x;
          gfloat y;
          gfloat dx;
          gfloat dy;

          g_print ("motion: %p:%i\n", box2d_child, id);

          x =  (event->motion.x);
          y =  (event->motion.y);

          clutter_actor_transform_stage_point (
            clutter_actor_get_parent (child_meta->actor),
            x, y,
            &x, &y);

           dx = x - priv->start_x;
           dy = y - priv->start_y;

           /* priv->mouse_joint may have been deleted while processing
            * events during clutter_actor_transform_stage_point */
           if (priv->mouse_joint)
             {
               ClutterVertex target = { x, y };
               clutter_box2d_mouse_joint_update_target (priv->mouse_joint, &target);
             }

           return TRUE;
        }
      break;

    case CLUTTER_BUTTON_RELEASE:
      if (priv->mouse_joint)
        {
          clutter_box2d_joint_destroy (priv->mouse_joint);
          priv->mouse_joint = NULL;

          priv->device_id = 111; /* this id should not be in use */
        }
      g_signal_handler_disconnect (stage, priv->captured_handler);
      priv->captured_handler = 0;

      return TRUE;

    default:
      break;
    }

  return FALSE;
}

