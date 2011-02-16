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

static void clutter_box2d_real_iterate (ClutterBox2D *box2d);


#define CLUTTER_BOX2D_GET_PRIVATE(obj)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_BOX2D, ClutterBox2DPrivate))

enum
{
  PROP_0,
  PROP_GRAVITY,
  PROP_SIMULATING,
  PROP_SCALE_FACTOR,
  PROP_TIME_STEP,
  PROP_ITERATIONS,
  PROP_SIMULATE_INACTIVE
};

static GObject * clutter_box2d_constructor (GType                  type,
                                            guint                  n_params,
                                            GObjectConstructParam *params);
static void      clutter_box2d_dispose     (GObject               *object);

static gboolean  clutter_box2d_iterate     (ClutterBox2D          *box2d);

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
start_simulation (ClutterBox2D *self)
{
  if (!self->priv->iterate_id)
    self->priv->iterate_id =
      g_timeout_add_full (CLUTTER_PRIORITY_REDRAW, self->priv->time_step,
                          (GSourceFunc)clutter_box2d_iterate,
                          self, NULL);
}

static void
stop_simulation (ClutterBox2D *self)
{
  if (self->priv->iterate_id)
    {
      g_source_remove (self->priv->iterate_id);
      self->priv->iterate_id = 0;
    }
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
            stop_simulation (box2d);
            start_simulation (box2d);
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
    case PROP_SIMULATE_INACTIVE:
      {
        box2d->priv->simulate_inactive = g_value_get_boolean (value);
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

    case PROP_SIMULATE_INACTIVE:
      g_value_set_boolean (value, box2d->priv->simulate_inactive);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_constructed (GObject *gobject)
{
  b2BodyDef bodyDef;
  ClutterBox2D *self = CLUTTER_BOX2D (gobject);
  ClutterBox2DPrivate *priv = self->priv;

  /* Create a new world with default gravity parameters.
   *
   * Note, that the simulation of inactive bodies is optional and default
   * on. It seems that enabling this feature (to not simulate them) works
   * really badly in certain cases (such as zero-gravity).
   */
  priv->world = new b2World (b2Vec2 (0.0f, 9.8f), !priv->simulate_inactive);

  priv->contact_listener = (_ClutterBox2DContactListener *)
    new __ClutterBox2DContactListener (self);

  priv->ground_body = priv->world->CreateBody (&bodyDef);

  start_simulation (self);
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
  gobject_class->constructed  = clutter_box2d_constructed;
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

  g_object_class_install_property (gobject_class,
                                   PROP_SIMULATE_INACTIVE,
                                   g_param_spec_boolean ("simulate-inactive",
                                                         "Simulate inactive",
                                                         "Whether to simulate inactive bodies",
                                                         TRUE,
                                                         static_cast<GParamFlags>(G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY)));
}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  ClutterBox2DPrivate *priv = self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  /* The Box2D manual recommends 10 iterations, but this isn't really
   * high enough to maintain a stable simulation with many stacked
   * actors.
   */
  priv->iterations = 10;
  priv->time_step  = 1000 / 60.f;
  priv->simulate_inactive = TRUE;

  priv->scale_factor     = 1/50.f;
  priv->inv_scale_factor = 1.f / priv->scale_factor;

  priv->actors = g_hash_table_new (g_direct_hash, g_direct_equal);
  priv->bodies = g_hash_table_new (g_direct_hash, g_direct_equal);
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

  stop_simulation (self);

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

  /* If the dirty flag is set, forcibly recreate the fixture */
  if (priv->dirty && box2d_child->priv->fixture)
    {
      box2d_child->priv->body->DestroyFixture (box2d_child->priv->fixture);
      box2d_child->priv->fixture = NULL;
    }

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

  ensure_shape (box2d, box2d_child);

  b2Vec2 position = body->GetPosition ();

  body->SetTransform (b2Vec2 (x * priv->scale_factor, y * priv->scale_factor),
                      rot / (180 / G_PI));

  SYNCLOG ("\t setxform: %d, %d, %f\n", x, y, rot);
}

static void
_clutter_box2d_sync_actor (ClutterBox2D *box2d, ClutterBox2DChild *box2d_child)
{
  gdouble rot;
  gfloat x, y, centre_x, centre_y;
  ClutterBox2DPrivate *priv = box2d->priv;
  ClutterActor *actor = CLUTTER_CHILD_META (box2d_child)->actor;
  b2Body       *body  = box2d_child->priv->body;

  if (!body)
    return;

  ensure_shape (box2d, box2d_child);

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

  rot = body->GetAngle () * (180 / G_PI);

  SYNCLOG ("setting actor position: ' %f %f angle: %lf\n", x, y, rot);

  clutter_actor_set_position (actor, x, y);
  clutter_actor_set_rotation (actor, CLUTTER_Z_AXIS,
                              body->GetAngle () * (180 / G_PI),
                              centre_x, centre_y, 0);

  /* Store the set values to know when to resync the body */
  box2d_child->priv->old_x = x;
  box2d_child->priv->old_y = y;
  box2d_child->priv->old_rot = rot;
}

static void
clutter_box2d_real_iterate (ClutterBox2D *box2d)
{
  ClutterBox2DPrivate *priv = box2d->priv;
  gint                 steps = priv->iterations;
  b2World             *world = priv->world;
  GList               *actors = g_hash_table_get_values (priv->actors);
  GList *iter;

  /* First we check for each actor the need for, and perform a sync
   * from the actor to the body, if necessary, before running simulation
   */
  for (iter = actors; iter; iter = g_list_next (iter))
    {
      gfloat x, y;
      gdouble rot;

      ClutterBox2DChild *box2d_child = (ClutterBox2DChild*) iter->data;
      ClutterActor *actor = CLUTTER_CHILD_META (box2d_child)->actor;

      clutter_actor_get_position (actor, &x, &y);
      rot = clutter_actor_get_rotation (actor, CLUTTER_Z_AXIS,
                                        NULL, NULL, NULL);

      if ((box2d_child->priv->old_x != x) ||
          (box2d_child->priv->old_y != y) ||
          (box2d_child->priv->old_rot != rot))
        _clutter_box2d_sync_body (box2d, box2d_child);
    }

  /* Iterate Box2D simulation of bodies */
  world->Step (priv->time_step / 1000.f, steps, steps);

  /* Synchronise actor to have geometrical sync with bodies */
  for (iter = actors; iter; iter = g_list_next (iter))
    {
      ClutterBox2DChild *box2d_child = (ClutterBox2DChild*) iter->data;
      _clutter_box2d_sync_actor (box2d, box2d_child);
    }
  g_list_free (actors);

  /* Reset the 'dirty' flag - all shapes would be recreated by the above
   * for-loop in the ensure_shape function.
   */
  priv->dirty = FALSE;

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

static gboolean
clutter_box2d_iterate (ClutterBox2D *box2d)
{
  ClutterBox2DPrivate *priv = box2d->priv;

  CLUTTER_BOX2D_GET_CLASS (box2d)->iterate (box2d);

  return TRUE;
}

void
clutter_box2d_set_gravity (ClutterBox2D        *box2d,
                           const ClutterVertex *gravity)
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

  if (!!simulating == !!priv->iterate_id)
    return;

  if (simulating)
    start_simulation (box2d);
  else
    stop_simulation (box2d);

  g_object_notify (G_OBJECT (box2d), "simulating");
}

gboolean
clutter_box2d_get_simulating (ClutterBox2D *box2d)
{
  ClutterBox2DPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), FALSE);

  priv = box2d->priv;

  return priv->iterate_id ? TRUE : FALSE;
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
      priv->dirty = TRUE;
      g_object_notify (G_OBJECT (box2d), "scale-factor");
    }
}

gfloat
clutter_box2d_get_scale_factor (ClutterBox2D *box2d)
{
  g_return_val_if_fail (CLUTTER_IS_BOX2D (box2d), 0.f);
  return box2d->priv->scale_factor;
}
