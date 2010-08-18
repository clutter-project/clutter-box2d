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

#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d-child.h"
#include "clutter-box2d-private.h"
#include "clutter-box2d.h"
#include "math.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "clutter-box2d-marshal.h"

/* defined in clutter-box2d.cpp */
void _clutter_box2d_sync_body (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child);

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

static void     clutter_box2d_child_dispose     (GObject *object);
static void     clutter_box2d_child_finalize    (GObject *object);
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
  return box2d_child->priv->body->IsBullet ();
}

static void
clutter_box2d_child_set_type2 (ClutterBox2DChild *box2d_child,
                               ClutterBox2DType   type)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D(clutter_child_meta_get_container (CLUTTER_CHILD_META(box2d_child)));
  b2World *world = box2d->priv->world;

  if (box2d_child->priv->type == type)
    return;

  if (box2d_child->priv->type != CLUTTER_BOX2D_NONE)
    {
      g_assert (box2d_child->priv->body);

      g_hash_table_remove (box2d->priv->bodies, box2d_child->priv->body);
      world->DestroyBody (box2d_child->priv->body);
      box2d_child->priv->body = NULL;
      box2d_child->priv->fixture = NULL;
      box2d_child->priv->type = CLUTTER_BOX2D_NONE;
    }

  if (type == CLUTTER_BOX2D_DYNAMIC ||
      type == CLUTTER_BOX2D_STATIC)
    {
      b2BodyDef bodyDef;

      bodyDef.linearDamping = 0.5f;
      bodyDef.angularDamping = 0.5f;


      SYNCLOG ("making an actor to be %s\n",
               type == CLUTTER_BOX2D_STATIC ? "static" : "dynamic");

      box2d_child->priv->type = type;

      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          bodyDef.type = b2_dynamicBody;
          box2d_child->priv->body = world->CreateBody (&bodyDef);
        }
      else if (type == CLUTTER_BOX2D_STATIC)
        {

          bodyDef.type = b2_staticBody;
          box2d_child->priv->body = world->CreateBody (&bodyDef);
        }
      _clutter_box2d_sync_body (box2d, box2d_child);

      g_hash_table_insert (box2d->priv->bodies, box2d_child->priv->body, box2d_child);
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
  if (box2d_child->priv->fixture)
    {
      ClutterBox2D *box2d = CLUTTER_BOX2D (clutter_child_meta_get_container (
                                           CLUTTER_CHILD_META (box2d_child)));
      box2d_child->priv->body->DestroyFixture (box2d_child->priv->fixture);
      box2d_child->priv->fixture = NULL;
      _clutter_box2d_sync_body (box2d, box2d_child);
    }
}

static void
clutter_box2d_child_set_manipulatable_internal (ClutterBox2DChild *box2d_child,
                                                ClutterActor      *child,
                                                gboolean           manipulatable)
{
  ClutterBox2DChildPrivate *priv = box2d_child->priv;

  if (priv->manipulatable == manipulatable)
    return;

  priv->manipulatable = manipulatable;

  if (priv->manipulatable)
    {
      priv->was_reactive = clutter_actor_get_reactive (child);
      clutter_actor_set_reactive (child, TRUE);
      priv->press_handler =
        g_signal_connect (child, "button-press-event",
                          G_CALLBACK (clutter_box2d_child_press), box2d_child);
    }
  else
    {
      if (!priv->was_reactive)
        clutter_actor_set_reactive (child, FALSE);

      g_signal_handler_disconnect (child, priv->press_handler);
      priv->press_handler = 0;

      if (priv->captured_handler)
        {
          g_signal_handler_disconnect (clutter_actor_get_stage (child),
                                       priv->captured_handler);
          priv->captured_handler = 0;
        }
    }

  g_object_notify (G_OBJECT (box2d_child), "manipulatable");
}

static void
clutter_box2d_child_set_is_bullet_internal (ClutterBox2DChild *box2d_child,
                                            gboolean           is_bullet)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (clutter_child_meta_get_container (
                                       CLUTTER_CHILD_META (box2d_child)));
  _clutter_box2d_sync_body (box2d, box2d_child);

  if (!box2d_child->priv->body ||
      box2d_child->priv->body->IsBullet () == is_bullet)
    return;

  box2d_child->priv->body->SetBullet (is_bullet);
  g_object_notify (G_OBJECT (box2d_child), "is-bullet");
}

static void
clutter_box2d_child_set_is_circle_internal (ClutterBox2DChild *box2d_child,
                                            gboolean           is_circle)
{
  if (box2d_child->priv->is_circle != is_circle)
    {
      box2d_child->priv->is_circle = is_circle;
      clutter_box2d_child_refresh_shape (box2d_child);
      g_object_notify (G_OBJECT (box2d_child), "is-circle");
    }
}

static void
clutter_box2d_child_set_density_internal (ClutterBox2DChild *box2d_child,
                                          gfloat             density)
{
  if (box2d_child->priv->density != density)
    {
      box2d_child->priv->density = density;
      clutter_box2d_child_refresh_shape (box2d_child);
      g_object_notify (G_OBJECT (box2d_child), "density");
    }
}

static void
clutter_box2d_child_set_friction_internal (ClutterBox2DChild *box2d_child,
                                           gfloat             friction)
{
  if (box2d_child->priv->friction != friction)
    {
      box2d_child->priv->friction = friction;
      clutter_box2d_child_refresh_shape (box2d_child);
      g_object_notify (G_OBJECT (box2d_child), "friction");
    }
}

static void
clutter_box2d_child_set_restitution_internal (ClutterBox2DChild *box2d_child,
                                              gfloat             restitution)
{
  if (box2d_child->priv->restitution != restitution)
    {
      box2d_child->priv->restitution = restitution;
      clutter_box2d_child_refresh_shape (box2d_child);
      g_object_notify (G_OBJECT (box2d_child), "restitution");
    }
}

static void
clutter_box2d_child_set_linear_velocity_internal (ClutterBox2DChild   *box2d_child,
                                                  const ClutterVertex *velocity)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (clutter_child_meta_get_container (
                                       CLUTTER_CHILD_META (box2d_child)));
  b2Vec2 b2velocity (velocity->x * box2d->priv->scale_factor,
                     velocity->y * box2d->priv->scale_factor);
  box2d_child->priv->body->SetLinearVelocity (b2velocity);
}

static void
clutter_box2d_child_set_angular_velocity_internal (ClutterBox2DChild *box2d_child,
                                                   gfloat             velocity)
{
  box2d_child->priv->body->SetAngularVelocity (velocity);
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
      clutter_box2d_child_set_manipulatable_internal (box2d_child,
                                                      clutter_child_meta_get_actor (child_meta),
                                                      g_value_get_boolean (value));
      break;

    case PROP_IS_BULLET:
      clutter_box2d_child_set_is_bullet_internal (box2d_child,
                                                  g_value_get_boolean (value));
      break;

    case PROP_IS_CIRCLE:
      clutter_box2d_child_set_is_circle_internal (box2d_child,
                                                  g_value_get_boolean (value));
      break;

    case PROP_DENSITY:
      clutter_box2d_child_set_density_internal (box2d_child,
                                                g_value_get_float (value));
      break;

    case PROP_FRICTION:
      clutter_box2d_child_set_friction_internal (box2d_child,
                                                 g_value_get_float (value));
      break;

    case PROP_RESTITUTION:
      clutter_box2d_child_set_restitution_internal (box2d_child,
                                                    g_value_get_float (value));
      break;

    case PROP_LINEAR_VELOCITY:
      clutter_box2d_child_set_linear_velocity_internal (box2d_child,
                                                        (ClutterVertex *)
                                                        g_value_get_boxed (value));
      break;

    case PROP_ANGULAR_VELOCITY:
      clutter_box2d_child_set_angular_velocity_internal (box2d_child,
                                                         g_value_get_float (value));
      break;

    case PROP_MODE:
      clutter_box2d_child_set_type2 (box2d_child, (ClutterBox2DType)g_value_get_int (value));

      break;

    case PROP_OUTLINE:
      {
        GValueArray *array;

        g_object_freeze_notify (gobject);

        if (box2d_child->priv->is_circle)
          {
            box2d_child->priv->is_circle = FALSE;
            g_object_notify (gobject, "is-circle");
          }

        g_free (box2d_child->priv->outline);
        g_free (box2d_child->priv->b2outline);
        box2d_child->priv->outline = NULL;
        box2d_child->priv->b2outline = NULL;
        box2d_child->priv->n_vertices = 0;

        array = (GValueArray *)g_value_get_boxed (value);
        if (array && (array->n_values > 2))
          {
            gint i;

            box2d_child->priv->outline = g_new0 (ClutterVertex, array->n_values);
            for (i = 0; i < array->n_values; i++)
              box2d_child->priv->outline[i] = *((ClutterVertex *)
                                          g_value_get_boxed (
                                            g_value_array_get_nth (array, i)));
            box2d_child->priv->n_vertices = array->n_values;
            box2d_child->priv->b2outline = (b2Vec2 *)
              g_malloc (sizeof (b2Vec2) * array->n_values);
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
        g_value_set_boolean (value, box2d_child->priv->body?
                                    box2d_child->priv->body->IsBullet ():FALSE);
      break;
    case PROP_IS_CIRCLE:
      g_value_set_boolean (value, box2d_child->priv->is_circle);
      break;
    case PROP_DENSITY:
      g_value_set_float (value, box2d_child->priv->density);
      break;
    case PROP_FRICTION:
      g_value_set_float (value, box2d_child->priv->friction);
      break;
    case PROP_RESTITUTION:
      g_value_set_float (value, box2d_child->priv->restitution);
      break;
    case PROP_LINEAR_VELOCITY:
      {
        ClutterVertex vertex = { 0, };

        if (box2d_child->priv->body)
          {
            ClutterBox2D *box2d = CLUTTER_BOX2D (clutter_child_meta_get_container (
                                                 CLUTTER_CHILD_META (box2d_child)));
            b2Vec2 velocity = box2d_child->priv->body->GetLinearVelocity();
            vertex.x = velocity.x / box2d->priv->scale_factor;
            vertex.y = velocity.y / box2d->priv->scale_factor;
            vertex.z = 0;
          }

        g_value_set_boxed (value, &vertex);
      }
      break;
    case PROP_ANGULAR_VELOCITY:
      g_value_set_float (value, box2d_child->priv->body?
                         box2d_child->priv->body->GetAngularVelocity():0.f);
      break;
    case PROP_MODE:
      g_value_set_int (value, box2d_child->priv->type);
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

  gobject_class->dispose      = clutter_box2d_child_dispose;
  gobject_class->finalize     = clutter_box2d_child_finalize;
  gobject_class->set_property = clutter_box2d_child_set_property;
  gobject_class->get_property = clutter_box2d_child_get_property;
  gobject_class->constructed  = clutter_box2d_child_constructed;

  box2d_child_signals[COLLISION] = g_signal_new ("collision",
                                 G_TYPE_FROM_CLASS (gobject_class),
                                 G_SIGNAL_RUN_LAST,
                                 0,
                                 NULL, NULL,
                                 _clutter_box2d_marshal_VOID__OBJECT,
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
                                   g_param_spec_float ("angular-velocity",
                                                       "Angular velocity",
                                                       "Angular velocity",
                                                       -G_MAXFLOAT, G_MAXFLOAT, 0.f,
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
  ClutterBox2DChildPrivate *priv = self->priv =
    CLUTTER_BOX2D_CHILD_GET_PRIVATE (self);
  priv->manipulatable = FALSE;
  priv->density = 7.0f;
  priv->friction = 0.4f;
  priv->restitution = 0.f;
}

static void
clutter_box2d_child_dispose (GObject *object)
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

  while (priv->joints)
    {
      clutter_box2d_joint_destroy ((ClutterBox2DJoint*)priv->joints->data);
    }

  G_OBJECT_CLASS (clutter_box2d_child_parent_class)->dispose (object);
}

static void
clutter_box2d_child_finalize (GObject *object)
{
  ClutterBox2DChildPrivate *priv = CLUTTER_BOX2D_CHILD (object)->priv;

  g_free (priv->outline);
  g_free (priv->b2outline);
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
      //g_print ("grab: %p:%i\n", actor, clutter_event_get_device_id (event));

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

          //g_print ("motion: %p:%i\n", box2d_child, id);

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

void
clutter_box2d_child_set_is_bullet (ClutterBox2D *box2d,
                                   ClutterActor *child,
                                   gboolean      is_bullet)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_is_bullet_internal (self, is_bullet);
}

gboolean
clutter_box2d_child_get_is_bullet (ClutterBox2D *box2d,
                                   ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return (self->priv->body ? self->priv->body->IsBullet () : FALSE);
}

void
clutter_box2d_child_set_is_circle (ClutterBox2D *box2d,
                                   ClutterActor *child,
                                   gboolean      is_circle)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_is_circle_internal (self, is_circle);
}

gboolean
clutter_box2d_child_get_is_circle (ClutterBox2D *box2d,
                                   ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->is_circle;
}

void
clutter_box2d_child_set_outline (ClutterBox2D        *box2d,
                                 ClutterActor        *child,
                                 const ClutterVertex *outline,
                                 guint                n_vertices)
{
  GObject *gobject;
  ClutterBox2DChild *self;
  ClutterBox2DChildPrivate *priv;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  self = clutter_box2d_get_child (box2d, child);
  if (!self)
    return;

  priv = self->priv;
  gobject = G_OBJECT (self);

  g_object_freeze_notify (gobject);

  if (priv->is_circle)
    {
      priv->is_circle = FALSE;
      g_object_notify (gobject, "is-circle");
    }

  g_free (priv->outline);
  g_free (priv->b2outline);
  priv->outline = NULL;
  priv->n_vertices = 0;

  if (outline && (n_vertices > 2))
    {
      priv->n_vertices = n_vertices;
      priv->outline = (ClutterVertex *)
        g_memdup (outline, sizeof (ClutterVertex) * n_vertices);
      priv->b2outline = (b2Vec2 *)g_malloc (sizeof (b2Vec2) * n_vertices);
    }

  g_object_notify (gobject, "outline");

  clutter_box2d_child_refresh_shape (self);

  g_object_thaw_notify (gobject);
}

const ClutterVertex *
clutter_box2d_child_get_outline (ClutterBox2D *box2d,
                                 ClutterActor *child,
                                 guint        *n_vertices)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), NULL);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), NULL);

  self = clutter_box2d_get_child (box2d, child);
  if (!self)
    return NULL;

  if (n_vertices)
    *n_vertices = self->priv->n_vertices;

  return self->priv->outline;
}

void
clutter_box2d_child_set_density (ClutterBox2D *box2d,
                                 ClutterActor *child,
                                 gfloat        density)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_density_internal (self, density);
}

gfloat
clutter_box2d_child_get_density (ClutterBox2D *box2d,
                                 ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0.f);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->density;
}

void
clutter_box2d_child_set_friction (ClutterBox2D *box2d,
                                  ClutterActor *child,
                                  gfloat        friction)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_friction_internal (self, friction);
}

gfloat
clutter_box2d_child_get_friction (ClutterBox2D *box2d,
                                  ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0.f);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->friction;
}

void
clutter_box2d_child_set_restitution (ClutterBox2D *box2d,
                                     ClutterActor *child,
                                     gfloat        restitution)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_restitution_internal (self, restitution);
}

gfloat
clutter_box2d_child_get_restitution (ClutterBox2D *box2d,
                                     ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0.f);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->restitution;
}

void
clutter_box2d_child_set_linear_velocity (ClutterBox2D        *box2d,
                                         ClutterActor        *child,
                                         const ClutterVertex *velocity)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_linear_velocity_internal (self, velocity);
}

void
clutter_box2d_child_get_linear_velocity (ClutterBox2D  *box2d,
                                         ClutterActor  *child,
                                         ClutterVertex *velocity)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if (!velocity)
    return;

  if ((self = clutter_box2d_get_child (box2d, child)) && self->priv->body)
    {
      b2Vec2 b2velocity = self->priv->body->GetLinearVelocity ();
      velocity->x = b2velocity.x * box2d->priv->inv_scale_factor;
      velocity->y = b2velocity.y * box2d->priv->inv_scale_factor;
      velocity->z = 0;
    }
  else
    *velocity = (ClutterVertex){ 0, 0, 0 };
}

void
clutter_box2d_child_set_angular_velocity (ClutterBox2D *box2d,
                                          ClutterActor *child,
                                          gfloat        velocity)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_angular_velocity_internal (self, velocity);
}

gfloat
clutter_box2d_child_get_angular_velocity (ClutterBox2D   *box2d,
                                          ClutterActor   *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0.f);

  if ((self = clutter_box2d_get_child (box2d, child)) && self->priv->body)
    return self->priv->body->GetAngularVelocity ();
  else
    return 0.f;
}

void
clutter_box2d_child_set_mode (ClutterBox2D     *box2d,
                              ClutterActor     *child,
                              ClutterBox2DType  mode)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_type2 (self, mode);
}

ClutterBox2DType
clutter_box2d_child_get_mode (ClutterBox2D *box2d,
                              ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), CLUTTER_BOX2D_NONE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), CLUTTER_BOX2D_NONE);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->type;
  else
    return CLUTTER_BOX2D_NONE;
}

void
clutter_box2d_child_set_manipulatable (ClutterBox2D *box2d,
                                       ClutterActor *child,
                                       gboolean      manipulatable)
{
  ClutterBox2DChild *self;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  if ((self = clutter_box2d_get_child (box2d, child)))
    clutter_box2d_child_set_manipulatable_internal (self, child, manipulatable);
}

gboolean
clutter_box2d_child_get_manipulatable (ClutterBox2D *box2d,
                                       ClutterActor *child)
{
  ClutterBox2DChild *self;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  if ((self = clutter_box2d_get_child (box2d, child)))
    return self->priv->manipulatable;
  else
    return FALSE;
}

