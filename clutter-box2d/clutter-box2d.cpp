/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of Box2D
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
#include "clutter-box2d-contact.h"
#include "clutter-box2d-private.h"
#include "math.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (ClutterBox2D, clutter_box2d, CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                             clutter_container_iface_init));

void _clutter_box2d_sync_body (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child);
static void clutter_box2d_real_iterate (ClutterBox2D *box2d, guint msecs);


#define CLUTTER_BOX2D_GET_PRIVATE(obj)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_BOX2D, ClutterBox2DPrivate))

enum
{
  PROP_0,
  PROP_GRAVITY,
  PROP_SIMULATING,
  PROP_SCALE_FACTOR,
  PROP_TIME_STEP,
  PROP_ITERATIONS
};

typedef enum 
{
  CLUTTER_BOX2D_JOINT_DEAD,     /* An associated actor has been killed off */
  CLUTTER_BOX2D_JOINT_DISTANCE,
  CLUTTER_BOX2D_JOINT_PRISMATIC,
  CLUTTER_BOX2D_JOINT_REVOLUTE,
  CLUTTER_BOX2D_JOINT_MOUSE
} ClutterBox2DJointType;


static GObject * clutter_box2d_constructor (GType                  type,
                                            guint                  n_params,
                                            GObjectConstructParam *params);
static void      clutter_box2d_dispose     (GObject               *object);

static void      clutter_box2d_iterate       (ClutterTimeline       *timeline,
                                              gint                   frame_num,
                                              gpointer               data);

ClutterBox2DChild *
clutter_box2d_get_child (ClutterBox2D *box2d,
                         ClutterActor *actor)
{
  ClutterChildMeta *meta;

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (box2d), actor);
  if (!meta)
    return NULL;

  return CLUTTER_BOX2D_CHILD (meta);
}

static void
clutter_box2d_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (clutter_box2d_parent_class)->paint (actor);

  /* XXX: enable drawing the shapes over the actors, as well as drawing
   *      lines for the joints
   */
}

static void
clutter_box2d_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (gobject);

  switch (prop_id)
    {
    case PROP_GRAVITY:
      {
        ClutterVertex *gravity = (ClutterVertex*) g_value_get_boxed (value);
        clutter_box2d_set_gravity (box2d, gravity);
      }
      break;
    case PROP_SIMULATING:
      {
        clutter_box2d_set_simulating (box2d, (gboolean)g_value_get_boolean (value));
      }
      break;
    case PROP_SCALE_FACTOR:
      {
        clutter_box2d_set_scale_factor (box2d, g_value_get_float (value));
      }
      break;
    case PROP_TIME_STEP:
      {
        gfloat time_step = g_value_get_float (value);
        if (box2d->priv->time_step != time_step)
          {
            box2d->priv->time_step = time_step;
            g_object_notify (gobject, "time-step");
          }
      }
      break;
    case PROP_ITERATIONS:
      {
        gint iterations = g_value_get_int (value);
        if (box2d->priv->iterations != iterations)
          {
            box2d->priv->iterations = iterations;
            g_object_notify (gobject, "iterations");
          }
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (gobject);

  switch (prop_id)
    {
    case PROP_GRAVITY:
      {
        ClutterVertex gravity;
        clutter_box2d_get_gravity (box2d, &gravity);
        g_value_set_boxed (value, &gravity);
      }
      break;

    case PROP_SIMULATING:
      g_value_set_boolean (value, clutter_box2d_get_simulating (box2d));
      break;

    case PROP_SCALE_FACTOR:
      g_value_set_float (value, clutter_box2d_get_scale_factor (box2d));
      break;

    case PROP_TIME_STEP:
      g_value_set_float (value, box2d->priv->time_step);
      break;

    case PROP_ITERATIONS:
      g_value_set_int (value, box2d->priv->iterations);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}


static void
clutter_box2d_class_init (ClutterBox2DClass *klass)
{
  GObjectClass          *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass     *actor_class   = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->dispose      = clutter_box2d_dispose;
  gobject_class->constructor  = clutter_box2d_constructor;
  gobject_class->set_property = clutter_box2d_set_property;
  gobject_class->get_property = clutter_box2d_get_property;
  actor_class->paint          = clutter_box2d_paint;
  klass->iterate              = clutter_box2d_real_iterate;

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DPrivate));

  /* gravity can only be set, not get */
  g_object_class_install_property (gobject_class,
                                   PROP_GRAVITY,
                                   g_param_spec_boxed ("gravity",
                                                       "Gravity",
                                                       "The gravity of ",
                                                       CLUTTER_TYPE_VERTEX,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class,
                                   PROP_SIMULATING,
                                   g_param_spec_boolean ("simulating",
                                                         "Simulating",
                                                         "Whether ClutterBox2D is performing physical simulation or not.",
                                                         TRUE,
                                                         static_cast<GParamFlags>(G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class,
                                   PROP_SCALE_FACTOR,
                                   g_param_spec_float ("scale-factor",
                                                       "Scale factor",
                                                       "The scaling factor of pixels to world units",
                                                       G_MINFLOAT, G_MAXFLOAT, 1/50.f,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class,
                                   PROP_TIME_STEP,
                                   g_param_spec_float ("time-step",
                                                       "Time step",
                                                       "The amount of time simulated in a physics step, in milliseconds",
                                                       1.f, 1000.f, 1000/60.f,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class,
                                   PROP_ITERATIONS,
                                   g_param_spec_float ("iterations",
                                                       "Iterations",
                                                       "The amount of iterations in a physics step",
                                                       1, G_MAXINT, 10,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE)));
}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  b2BodyDef bodyDef;
  ClutterBox2DPrivate *priv = self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  /* Create a new world with default gravity parameters and allowing inactive
   * bodies to not be simulated (improves performance).
   */
  priv->world = new b2World (b2Vec2 (0.0f, 9.8f), true);

  /* The Box2D manual recommends 10 iterations, but this isn't really
   * high enough to maintain a stable simulation with many stacked
   * actors.
   */
  priv->iterations = 10;
  priv->time_step  = 1000 / 60.f;

  priv->scale_factor     = 1/50.f;
  priv->inv_scale_factor = 1.f / priv->scale_factor;

  priv->actors = g_hash_table_new (g_direct_hash, g_direct_equal);
  priv->bodies = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* we make the timeline play continously to have a constant source
   * of new-frame events as long as the timeline is playing.
   */
  priv->timeline = clutter_timeline_new (1000);
  g_object_set (priv->timeline, "loop", TRUE, NULL);
  g_signal_connect (priv->timeline, "new-frame",
                    G_CALLBACK (clutter_box2d_iterate), self);

  priv->contact_listener = (_ClutterBox2DContactListener *)
    new __ClutterBox2DContactListener (self);

  priv->ground_body = priv->world->CreateBody (&bodyDef);
}

ClutterActor *
clutter_box2d_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CLUTTER_TYPE_BOX2D, NULL));
}

static GObject *
clutter_box2d_constructor (GType                  type,
                           guint                  n_params,
                           GObjectConstructParam *params)
{
  GObject              *object;

  object = G_OBJECT_CLASS (clutter_box2d_parent_class)->constructor (
    type, n_params, params);

  return object;
}

static void
clutter_box2d_dispose (GObject *object)
{
  ClutterBox2D        *self = CLUTTER_BOX2D (object);
  ClutterBox2DPrivate *priv = self->priv;

  G_OBJECT_CLASS (clutter_box2d_parent_class)->dispose (object);

  if (priv->timeline)
    {
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }
  if (priv->actors)
    {
      g_hash_table_destroy (priv->actors);
      priv->actors = NULL;
    }
  if (priv->bodies)
    {
      g_hash_table_destroy (priv->bodies);
      priv->bodies = NULL;
    }

  if (priv->contact_listener)
    {
      delete (__ClutterBox2DContactListener *)priv->contact_listener;
      priv->contact_listener = NULL;
    }
}


static void
clutter_box2d_create_child_meta (ClutterContainer *container,
                                 ClutterActor     *actor)

{
  ClutterChildMeta  *child_meta;
  ClutterBox2DChild *box2d_child;

  ClutterBox2DPrivate *priv = CLUTTER_BOX2D (container)->priv;

  child_meta = CLUTTER_CHILD_META (
    g_object_new (CLUTTER_TYPE_BOX2D_CHILD,
                  "container", container,
                  "actor", actor,
                  NULL));

  box2d_child = CLUTTER_BOX2D_CHILD (child_meta);
  box2d_child->priv->world = priv->world;

  g_hash_table_insert (priv->actors, actor, child_meta);
}

static void
clutter_box2d_destroy_child_meta (ClutterContainer *box2d,
                                  ClutterActor     *actor)
{
  gboolean manipulatable;
  ClutterBox2DChild *box2d_child =
     CLUTTER_BOX2D_CHILD (clutter_container_get_child_meta ( box2d, actor));
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D (box2d)->priv;

  g_assert (box2d_child->priv->world);

  if (box2d_child->priv->manipulatable)
    g_object_set (box2d_child, "manipulatable", FALSE, NULL);

  if (box2d_child->priv->body)
    box2d_child->priv->world->DestroyBody (box2d_child->priv->body);

  g_hash_table_remove (priv->actors, actor);
  g_hash_table_remove (priv->bodies, box2d_child->priv->body);
}

static ClutterChildMeta *
clutter_box2d_get_child_meta (ClutterContainer *container,
                              ClutterActor     *actor)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D (container)->priv;

  return CLUTTER_CHILD_META (g_hash_table_lookup (priv->actors, actor));
}

static void clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->child_meta_type = CLUTTER_TYPE_BOX2D_CHILD;
  iface->create_child_meta = clutter_box2d_create_child_meta;
  iface->destroy_child_meta = clutter_box2d_destroy_child_meta;   
  iface->get_child_meta = clutter_box2d_get_child_meta;
}

/* make sure that the shape attached to the body matches the clutter realms
 * idea of the shape.
 */
static inline void
ensure_shape (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child)
{
  ClutterBox2DPrivate *priv = box2d->priv;

  if (box2d_child->priv->fixture == NULL)
    {
      gfloat width, height;
      b2Shape *shape;
      b2FixtureDef fixture;
      b2CircleShape circle;
      b2PolygonShape polygon;
      ClutterChildMeta *meta = CLUTTER_CHILD_META (box2d_child);

      clutter_actor_get_size (meta->actor, &width, &height);

      if (box2d_child->priv->is_circle)
        {

          circle.m_radius = MIN (width, height) * 0.5 * priv->scale_factor;
          shape = &circle;
        }
      else if (box2d_child->priv->outline)
        {
          gint i;
          b2Vec2* b2outline = box2d_child->priv->b2outline;
          ClutterVertex *vertices = box2d_child->priv->outline;

          for (i = 0; i < box2d_child->priv->n_vertices; i++)
            b2outline[i].Set(vertices[i].x * width * priv->scale_factor,
                             vertices[i].y * height * priv->scale_factor);
          polygon.Set(b2outline, i);
          shape = &polygon;
        }
      else
        {
          polygon.SetAsBox (width * 0.5 * priv->scale_factor,
                            height * 0.5 * priv->scale_factor,
                            b2Vec2 (width * 0.5 * priv->scale_factor,
                            height * 0.5 * priv->scale_factor), 0);
          shape = &polygon;
        }

      fixture.shape = shape;
      fixture.friction = box2d_child->priv->friction;
      fixture.density = box2d_child->priv->density;
      fixture.restitution = box2d_child->priv->restitution;

      box2d_child->priv->fixture =
        box2d_child->priv->body->CreateFixture (&fixture);
    }
  else
    {
      /*XXX: recreate shape on every ensure? */
    }
}


/* Synchronise the state of the Box2D body with the
 * current geomery of the actor, only really do it if
 * we differ more than a certain delta to avoid disturbing
 * the physics computation
 */
void
_clutter_box2d_sync_body (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child)
{
  gint x, y;
  gdouble rot;

  ClutterBox2DPrivate *priv = box2d->priv;
  ClutterActor *actor = CLUTTER_CHILD_META (box2d_child)->actor;
  b2Body       *body  = box2d_child->priv->body;

  if (!body)
    return;

  rot = clutter_actor_get_rotation (CLUTTER_CHILD_META (box2d_child)->actor,
                                    CLUTTER_Z_AXIS, NULL, NULL, NULL);


  x = clutter_actor_get_x (actor);
  y = clutter_actor_get_y (actor);

  if (box2d_child->priv->is_circle)
    {
      gfloat radius = MIN (clutter_actor_get_width (actor),
                           clutter_actor_get_height (actor)) / 2.f;
      x += radius;
      y += radius;
    }

  b2Vec2 position = body->GetPosition ();

  ensure_shape (box2d, box2d_child);

  if (fabs (x * priv->scale_factor - (position.x)) > 0.1 ||
      fabs (y * priv->scale_factor - (position.y)) > 0.1 ||
      fabs (body->GetAngle()*(180/3.1415) - rot) > 2.0
      )
    {
      body->SetTransform (b2Vec2 (x * priv->scale_factor, y * priv->scale_factor),
                          rot / (180 / 3.1415));

      SYNCLOG ("\t setxform: %d, %d, %f\n", x, y, rot);
    }
}

/* Synchronise actor geometry from body, rounding erorrs introduced here should
 * be smaller than the threshold accepted when sync_body is being run to avoid
 * introducing errors.
 */
static void
_clutter_box2d_sync_actor (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child)
{
  gfloat x, y, centre_x, centre_y;
  ClutterBox2DPrivate *priv = box2d->priv;
  ClutterActor *actor = CLUTTER_CHILD_META (box2d_child)->actor;
  b2Body       *body  = box2d_child->priv->body;

  if (!body)
    return;

  x = body->GetPosition ().x * priv->inv_scale_factor;
  y = body->GetPosition ().y * priv->inv_scale_factor;

  if (box2d_child->priv->is_circle)
    {
      gfloat width = clutter_actor_get_width (actor);
      gfloat height = clutter_actor_get_height (actor);
      gfloat radius = MIN (width, height) / 2.f;

      x -= radius;
      y -= radius;

      centre_x = width / 2.f;
      centre_y = height / 2.f;
    }
  else
    {
      centre_x = 0;
      centre_y = 0;
    }

  clutter_actor_set_position (actor, x, y);

  SYNCLOG ("setting actor position: ' %f %f angle: %f\n",
           x, y,
           body->GetAngle () * (180 / 3.1415));

  clutter_actor_set_rotation (actor, CLUTTER_Z_AXIS,
                              body->GetAngle () * (180 / 3.1415),
                              centre_x, centre_y, 0);
}

static void
clutter_box2d_real_iterate (ClutterBox2D *box2d, guint msecs)
{
  ClutterBox2DPrivate *priv = box2d->priv;
  gint                 steps = priv->iterations;
  b2World             *world = priv->world;

  {
    GList *actors = g_hash_table_get_values (priv->actors);
    GList *iter;

    /* First we check for each actor the need for, and perform a sync
     * from the actor to the body before running simulation
     */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DChild *box2d_child = (ClutterBox2DChild*) iter->data;
        _clutter_box2d_sync_body (box2d, box2d_child);
      }

    if (msecs == 0)
      return;

    /* Iterate Box2D simulation of bodies */

    /* We do multiple iterations trying to keep up with
     * priv->time_step (60fps by default). We start
     * slowing the simulation when more than 4 frames are
     * skipped (so by default, when the framerate drops below
     * 15fps).
     */
    priv->time_delta = MIN (priv->time_delta + msecs, priv->time_step * 4.f);
    while (priv->time_delta > priv->time_step)
      {
        world->Step (priv->time_step / 1000.f, steps, steps);
        priv->time_delta -= priv->time_step;
      }


    /* syncronize actor to have geometrical sync with bodies */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DChild *box2d_child = (ClutterBox2DChild*) iter->data;
        _clutter_box2d_sync_actor (box2d, box2d_child);
      }
    g_list_free (actors);

    /* Process list of collisions and emit signals for any actors with
     * a registered callback. */
    for (iter = priv->collisions; iter; iter = g_list_next (iter))
      {
        ClutterBox2DCollision  *collision;
        ClutterBox2DChild      *box2d_child1, *box2d_child2;
        ClutterBox2DChildClass *klass;

        collision = CLUTTER_BOX2D_COLLISION (iter->data);

        box2d_child1 = clutter_box2d_get_child (box2d, collision->actor1);

        if (box2d_child1)
          {
            klass = CLUTTER_BOX2D_CHILD_CLASS (G_OBJECT_GET_CLASS (box2d_child1));
            g_signal_emit_by_name (box2d_child1, "collision", collision);
          }

        box2d_child2 = clutter_box2d_get_child (box2d, collision->actor2);

        if (box2d_child2)
          {
            klass = CLUTTER_BOX2D_CHILD_CLASS (G_OBJECT_GET_CLASS (box2d_child2));
            g_signal_emit_by_name (box2d_child2, "collision", collision);
          }

        g_object_unref (collision);
      }
    g_list_free (priv->collisions);
    priv->collisions = NULL;
  }
}

static void
clutter_box2d_iterate (ClutterTimeline *timeline,
                       gint             frame_num,
                       gpointer         data)
{
  ClutterBox2D        *box2d = CLUTTER_BOX2D (data);
  ClutterBox2DPrivate *priv = box2d->priv;
  guint                msecs;

  if (priv->first_iteration)
    {
      msecs = 0;
      priv->time_delta = 0;
      priv->first_iteration = FALSE;
    }
  else
    msecs = clutter_timeline_get_delta (timeline);

  CLUTTER_BOX2D_GET_CLASS (box2d)->iterate (box2d, msecs);
}

void
clutter_box2d_set_gravity (ClutterBox2D  *box2d,
                           ClutterVertex *gravity)
{
  ClutterVertex old_gravity;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));

  clutter_box2d_get_gravity (box2d, &old_gravity);
  if ((gravity->x != old_gravity.x) ||
      (gravity->y != old_gravity.y))
    {
      b2Vec2 b2gravity = b2Vec2 (gravity->x, gravity->y);

      box2d->priv->world->SetGravity (b2gravity);

      g_object_notify (G_OBJECT (box2d), "gravity");
    }
}

void
clutter_box2d_get_gravity (ClutterBox2D  *box2d,
                           ClutterVertex *gravity)
{
  b2Vec2 b2gravity;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));

  if (!gravity)
    return;

  b2gravity = box2d->priv->world->GetGravity ();

  gravity->x = b2gravity.x;
  gravity->y = b2gravity.y;
  gravity->z = 0;
}

void
clutter_box2d_set_simulating (ClutterBox2D  *box2d,
                              gboolean       simulating)
{
  ClutterBox2DPrivate *priv;
  gboolean currently_simulating;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));

  priv = box2d->priv;

  currently_simulating = clutter_timeline_is_playing (priv->timeline);
  if (simulating == currently_simulating)
    return;

  if (simulating)
    {
      priv->first_iteration = TRUE;
      clutter_timeline_start (priv->timeline);
    }
  else
    {
      clutter_timeline_stop (priv->timeline);
    }

  g_object_notify (G_OBJECT (box2d), "simulating");
}

gboolean
clutter_box2d_get_simulating (ClutterBox2D *box2d)
{
  ClutterBox2DPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), FALSE);

  priv = box2d->priv;

  return clutter_timeline_is_playing (priv->timeline);
}

void
clutter_box2d_set_scale_factor (ClutterBox2D *box2d,
                                gfloat        scale_factor)
{
  ClutterBox2DPrivate *priv;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));

  priv = box2d->priv;
  if (priv->scale_factor != scale_factor)
    {
      priv->scale_factor = scale_factor;
      priv->inv_scale_factor = 1.f/scale_factor;
      g_object_notify (G_OBJECT (box2d), "scale-factor");
    }
}

gfloat
clutter_box2d_get_scale_factor (ClutterBox2D *box2d)
{
  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  return box2d->priv->scale_factor;
}
