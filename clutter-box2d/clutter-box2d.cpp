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

#define SCALE_FACTOR        0.05
#define INV_SCALE_FACTOR    (1.0/SCALE_FACTOR)

#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "clutter-box2d-actor.h"
#include "clutter-box2d-contact.h"
#include "math.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (ClutterBox2D, clutter_box2d, CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                             clutter_container_iface_init));

void _clutter_box2d_sync_body (ClutterBox2DActor *box2d_actor);
static void clutter_box2d_real_iterate (ClutterBox2D *box2d, guint msecs);


#define CLUTTER_BOX2D_GET_PRIVATE(obj)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_BOX2D, ClutterBox2DPrivate))

enum
{
  PROP_0,
  PROP_GRAVITY,
  PROP_SIMULATING
};

struct _ClutterBox2DPrivate
{
  gdouble          fps;         /* The framerate simulation is running at        */
  gint             iterations;  /* number of engine iterations per processing */
  gfloat           max_delta;   /* Largest time delta to simulate             */
  ClutterTimeline *timeline;    /* The timeline driving the simulation        */
  gboolean         first_iteration;
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
        b2Vec2 b2gravity = b2Vec2( (gravity->x),
                                   (gravity->y));
        ((b2World*)box2d->world)->SetGravity (b2gravity);
      }
      break;
    case PROP_SIMULATING:
      {
        clutter_box2d_set_simulating (box2d, (gboolean)g_value_get_boolean (value));
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
    case PROP_SIMULATING:
        g_value_set_boolean (value,
                             clutter_box2d_get_simulating (box2d));
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
                                                       G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_SIMULATING,
                                   g_param_spec_boolean ("simulating",
                                                         "Simulating",
                                                         "Whether ClutterBox2D is performing physical simulation or not.",
                                                         TRUE,
                                                         static_cast<GParamFlags>(G_PARAM_READWRITE)));

}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  bool                  doSleep;

  b2AABB                worldAABB;

  ClutterBox2DPrivate *priv = self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  /*  these magic numbers are the extent of the world,
   *  should be set large enough to contain most geometry,
   *  not sure how large this can be without impacting performance.
   */
  worldAABB.lowerBound.Set (-650.0f, -650.0f);
  worldAABB.upperBound.Set (650.0f, 650.0f);

  self->world = new b2World (worldAABB, /*gravity:*/ b2Vec2 (0.0f, 30.0f),
                             doSleep = false);

  priv->fps        = 25;
  priv->iterations = 50;
  priv->max_delta  = 16;

  self->actors = g_hash_table_new (g_direct_hash, g_direct_equal);
  self->bodies = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* we make the timeline play continously to have a constant source
   * of new-frame events as long as the timeline is playing.
   */
  priv->timeline = clutter_timeline_new (1000);
  g_object_set (priv->timeline, "loop", TRUE, NULL);
  g_signal_connect (priv->timeline, "new-frame",
                    G_CALLBACK (clutter_box2d_iterate), self);

  self->contact_listener = (_ClutterBox2DContactListener *)
    new __ClutterBox2DContactListener (self);
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
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  G_OBJECT_CLASS (clutter_box2d_parent_class)->dispose (object);

  if (priv->timeline)
    {
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }
  if (self->actors)
    {
      g_hash_table_destroy (self->actors);
      self->actors = NULL;
    }
  if (self->bodies)
    {
      g_hash_table_destroy (self->bodies);
      self->bodies = NULL;
    }

  if (self->contact_listener)
    {
      delete (__ClutterBox2DContactListener *)self ->contact_listener;
      self ->contact_listener = NULL;
    }
}


static void
clutter_box2d_create_child_meta (ClutterContainer *container,
                                 ClutterActor     *actor)

{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;

  child_meta = CLUTTER_CHILD_META (
    g_object_new (CLUTTER_TYPE_BOX2D_ACTOR,
                  "container", container,
                  "actor", actor,
                  NULL));

  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  box2d_actor->world = (b2World*)(CLUTTER_BOX2D (container)->world);

  g_hash_table_insert (CLUTTER_BOX2D (container)->actors, actor, child_meta);
}

static void
clutter_box2d_destroy_child_meta (ClutterContainer *box2d,
                                  ClutterActor     *actor)
{
  gboolean manipulatable;
  ClutterBox2DActor *box2d_actor =
     CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta ( box2d, actor));

  g_assert (box2d_actor->world);

  g_object_get (box2d_actor, "manipulatable", &manipulatable, NULL);
  if (manipulatable)
    g_object_set (box2d_actor, "manipulatable", FALSE, NULL);

  if (box2d_actor->body)
    box2d_actor->world->DestroyBody (box2d_actor->body);

  g_hash_table_remove (CLUTTER_BOX2D (box2d)->actors, actor);
  g_hash_table_remove (CLUTTER_BOX2D (box2d)->bodies, box2d_actor->body);
}

static ClutterChildMeta *
clutter_box2d_get_child_meta (ClutterContainer *container,
                              ClutterActor     *actor)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (container);

  return CLUTTER_CHILD_META (g_hash_table_lookup (box2d->actors, actor));
}

static void clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->child_meta_type = CLUTTER_TYPE_BOX2D_ACTOR;
  iface->create_child_meta = clutter_box2d_create_child_meta;
  iface->destroy_child_meta = clutter_box2d_destroy_child_meta;   
  iface->get_child_meta = clutter_box2d_get_child_meta;
}

/* make sure that the shape attached to the body matches the clutter realms
 * idea of the shape.
 */
static inline void
ensure_shape (ClutterBox2DActor *box2d_actor)
{
  if (box2d_actor->shape == NULL)
    {
      gfloat width, height;
      b2ShapeDef *shapeDef;
      b2CircleDef circleDef;
      b2PolygonDef polygonDef;
      ClutterChildMeta *meta = CLUTTER_CHILD_META (box2d_actor);

      clutter_actor_get_size (meta->actor, &width, &height);

      if (box2d_actor->is_circle)
        {

          circleDef.radius = MIN (width, height) * 0.5 * SCALE_FACTOR;
          shapeDef = &circleDef;
        }
      else if (box2d_actor->outline)
        {
          gint i;
          ClutterVertex *vertices = box2d_actor->outline;

          for (i = 0;
               (i == 0) ||
               (!clutter_vertex_equal (&vertices[i], &vertices[0]));
               i++)
            polygonDef.vertices[i].Set(vertices[i].x * width * SCALE_FACTOR,
                                       vertices[i].y * height * SCALE_FACTOR);
          polygonDef.vertexCount = i;
          shapeDef = &polygonDef;
        }
      else
        {
          polygonDef.SetAsBox (width * 0.5 * SCALE_FACTOR,
                               height * 0.5 * SCALE_FACTOR,
                               b2Vec2 (width * 0.5 * SCALE_FACTOR,
                               height * 0.5 * SCALE_FACTOR), 0);
          shapeDef = &polygonDef;
        }

      shapeDef->density = box2d_actor->density;
      shapeDef->friction = box2d_actor->friction;
      shapeDef->restitution = box2d_actor->restitution;
      box2d_actor->shape = box2d_actor->body->CreateShape (shapeDef);
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
_clutter_box2d_sync_body (ClutterBox2DActor *box2d_actor)
{
  gint x, y;
  gdouble rot;

  ClutterActor *actor = CLUTTER_CHILD_META (box2d_actor)->actor;
  b2Body       *body  = box2d_actor->body;

  if (!body)
    return;

  rot = clutter_actor_get_rotation (CLUTTER_CHILD_META (box2d_actor)->actor,
                                    CLUTTER_Z_AXIS, NULL, NULL, NULL);


  x = clutter_actor_get_x (actor);
  y = clutter_actor_get_y (actor);

  if (box2d_actor->is_circle)
    {
      gfloat radius = MIN (clutter_actor_get_width (actor),
                           clutter_actor_get_height (actor)) / 2.f;
      x += radius;
      y += radius;
    }

  b2Vec2 position = body->GetPosition ();

  ensure_shape (box2d_actor);

  if (fabs (x * SCALE_FACTOR - (position.x)) > 0.1 ||
      fabs (y * SCALE_FACTOR - (position.y)) > 0.1 ||
      fabs (body->GetAngle()*(180/3.1415) - rot) > 2.0
      )
    {
      body->SetXForm (b2Vec2 (x * SCALE_FACTOR, y * SCALE_FACTOR),
                      rot / (180 / 3.1415));

      SYNCLOG ("\t setxform: %d, %d, %f\n", x, y, rot);
    }
}

/* Synchronise actor geometry from body, rounding erorrs introduced here should
 * be smaller than the threshold accepted when sync_body is being run to avoid
 * introducing errors.
 */
static void
_clutter_box2d_sync_actor (ClutterBox2DActor *box2d_actor)
{
  gfloat x, y, centre_x, centre_y;
  ClutterActor *actor = CLUTTER_CHILD_META (box2d_actor)->actor;
  b2Body       *body  = box2d_actor->body;

  if (!body)
    return;

  x = body->GetPosition ().x * INV_SCALE_FACTOR;
  y = body->GetPosition ().y * INV_SCALE_FACTOR;

  if (box2d_actor->is_circle)
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
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  gint                 steps = priv->iterations;
  b2World             *world = (b2World*)(box2d->world);

  {
    GList *actors = g_hash_table_get_values (box2d->actors);
    GList *iter;

    /* First we check for each actor the need for, and perform a sync
     * from the actor to the body before running simulation
     */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *box2d_actor = (ClutterBox2DActor*) iter->data;
        _clutter_box2d_sync_body (box2d_actor);
      }

    if (msecs == 0)
      return;

    /* Iterate Box2D simulation of bodies */

    /* Scale the amount of iteration steps with the time delta to maintain
     * a stable system. Put a cap on this value though, so we don't end up
     * with a possibly infinitely-increasing lag as the scale tries to
     * compensate.
     *
     * TODO: Perhaps some heuristic to decrease the accuracy of the
     *       simulation if the CPU can't keep up?
     */
    steps = MIN (MAX (1, (gint)(steps * msecs / priv->max_delta)),
                 priv->iterations * 10);
    world->Step (msecs / 1000.0, steps, steps);


    /* syncronize actor to have geometrical sync with bodies */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *box2d_actor = (ClutterBox2DActor*) iter->data;
        _clutter_box2d_sync_actor (box2d_actor);
      }
    g_list_free (actors);

    /* Process list of collisions and emit signals for any actors with
     * a registered callback. */
    for (iter = box2d->collisions; iter; iter = g_list_next (iter))
      {
        ClutterBox2DCollision  *collision;
        ClutterBox2DActor      *box2d_actor1, *box2d_actor2;
        ClutterBox2DActorClass *klass;

        collision = CLUTTER_BOX2D_COLLISION (iter->data);

        box2d_actor1 = clutter_box2d_get_actor (box2d, collision->actor1);

        if (box2d_actor1)
          {
            klass = CLUTTER_BOX2D_ACTOR_CLASS (G_OBJECT_GET_CLASS (box2d_actor1));
            g_signal_emit_by_name (box2d_actor1, "collision", collision);
          }

        box2d_actor2 = clutter_box2d_get_actor (box2d, collision->actor2);

        if (box2d_actor2)
          {
            klass = CLUTTER_BOX2D_ACTOR_CLASS (G_OBJECT_GET_CLASS (box2d_actor2));
            g_signal_emit_by_name (box2d_actor2, "collision", collision);
          }

        g_object_unref (collision);
      }
    g_list_free (box2d->collisions);
    box2d->collisions = NULL;
  }
}

static void
clutter_box2d_iterate (ClutterTimeline *timeline,
                       gint             frame_num,
                       gpointer         data)
{
  ClutterBox2D        *box2d = CLUTTER_BOX2D (data);
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  guint                msecs;

  if (priv->first_iteration)
    {
      msecs = 0;
      priv->first_iteration = FALSE;
    }
  else
    msecs = clutter_timeline_get_delta (timeline);

  CLUTTER_BOX2D_GET_CLASS (box2d)->iterate (box2d, msecs);
}

void
clutter_box2d_set_simulating (ClutterBox2D  *box2d,
                              gboolean       simulating)
{
  ClutterBox2DPrivate *priv;
  gboolean currently_simulating;

  g_return_if_fail (CLUTTER_IS_BOX2D (box2d));

  priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);

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

  priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);

  return clutter_timeline_is_playing (priv->timeline);
}
