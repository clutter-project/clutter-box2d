/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of ChipMunk.
 *
 * Copyright 2007 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#define SYNCLOG(argv...)    if (0) g_print (argv)

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "math.h"

G_DEFINE_TYPE (ClutterBox2D, clutter_box2d, CLUTTER_TYPE_GROUP);

#define CLUTTER_BOX2D_GET_PRIVATE(obj)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                CLUTTER_TYPE_BOX2D, \
                                ClutterBox2DPrivate))

struct _ClutterBox2DPrivate
{
  b2World         *world;
  GHashTable      *actors;
  GHashTable      *bodies; /* mapping to space actors */

  gdouble          fps;
  gint             iterations;

  ClutterTimeline *timeline;

};

typedef struct _ClutterBox2DActor ClutterBox2DActor;
struct _ClutterBox2DActor
{
  ClutterBox2DType  type;
  ClutterBox2D     *clutter_box2d;
  ClutterActor     *actor;

  b2Body           *body;
  b2World          *world;
  b2Shape          *shape;

  gdouble scale_x; /* cached values of geometry, kept to */
  gdouble scale_y; /* see if the shape needs recomputation */
  gint    width;
  gint    height;
  gint    contacts;
};

struct _ClutterBox2DSpring
{
  ClutterBox2D      *clutter_box2d;
  ClutterBox2DActor *actor_a;
  ClutterBox2DActor *actor_b;
  gdouble            x_a;
  gdouble            y_a;
  gdouble            x_b;
  gdouble            y_b;
  gdouble            rest_length;
  gdouble            spring_foo;
  gdouble            damping;
  gdouble            dt;
};

static ClutterBox2DActor *
clutter_box2d_get_actor (ClutterBox2D   *space,
                         ClutterActor   *actor);

static void
sync_body (ClutterBox2DActor *space_actor);
static void
sync_actor (ClutterBox2DActor *space_actor);

static void
on_space_actor_weak_notify (gpointer data,
                            GObject *where_the_actor_was)
{
  ClutterBox2DActor *space_actor = (ClutterBox2DActor*) data;

  g_assert (space_actor->world);

  g_hash_table_remove (CLUTTER_BOX2D_GET_PRIVATE (
                         space_actor->clutter_box2d)->actors,
                       space_actor->actor);
  space_actor->actor = NULL;
  g_free (space_actor);
}

ClutterBox2DActor *
clutter_box2d_get_actor (ClutterBox2D *space,
                         ClutterActor *actor)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (space);
  ClutterBox2DActor   *space_actor;

  g_assert (CLUTTER_IS_ACTOR (actor));

  space_actor = (ClutterBox2DActor*) g_hash_table_lookup (priv->actors, actor);

  if (space_actor)
    return space_actor;

  space_actor = (ClutterBox2DActor*) (g_malloc0 (sizeof (ClutterBox2DActor)));

  space_actor->type          = CLUTTER_BOX2D_UNINITIALIZED;
  space_actor->clutter_box2d = space;
  space_actor->actor         = actor;
  space_actor->world         = priv->world;

  g_object_weak_ref (G_OBJECT (actor), on_space_actor_weak_notify, space_actor);

  g_hash_table_insert (priv->actors, actor, space_actor);

  return space_actor;
}

ClutterBox2DActor *
clutter_box2d_actor (ClutterActor *actor)
{
  ClutterBox2D *space = CLUTTER_BOX2D (clutter_actor_get_parent (actor));

  return clutter_box2d_get_actor (space, actor);
}

ClutterBox2DType
clutter_box2d_actor_get_type (ClutterBox2D   *space,
                              ClutterActor   *actor)
{
  ClutterBox2DActor *space_actor = clutter_box2d_get_actor (space, actor);

  return space_actor->type;
}

static inline void
ensure_shape (ClutterBox2DActor *space_actor)
{
  /* make sure that the shape attached to the body matches the clutter realms
   * idea of the shape.
   */

  if (space_actor->shape == NULL)
    {
      b2PolygonDef shapeDef;
      gint         width, height;
      gdouble      rot;

      width  = clutter_actor_get_width (space_actor->actor);
      height = clutter_actor_get_height (space_actor->actor);
      rot    = clutter_actor_get_rotation (space_actor->actor,
                                           CLUTTER_Z_AXIS, NULL, NULL, NULL);

      shapeDef.SetAsBox (width * 0.5, height * 0.5,
                         b2Vec2 (width * 0.5, height * 0.5), 0);
      shapeDef.density   = 0.6f;
      space_actor->shape = space_actor->body->CreateShape (&shapeDef);
    }
}

void
clutter_box2d_actor_set_type (ClutterBox2D      *space,
                              ClutterActor      *actor,
                              ClutterBox2DType   type)
{
  ClutterBox2DActor *space_actor = clutter_box2d_get_actor (space, actor);

  if (space_actor->type == type)
    return;
  g_assert (space_actor->type == CLUTTER_BOX2D_UNINITIALIZED);


  if (type == CLUTTER_BOX2D_DYNAMIC ||
      type == CLUTTER_BOX2D_STATIC)
    {
      SYNCLOG ("setting an actor to be %s\n",
               type == CLUTTER_BOX2D_STATIC ? "static" : "dynamic");
      
      gint width  = clutter_actor_get_width (space_actor->actor);
      gint height = clutter_actor_get_height (space_actor->actor);

      space_actor->type = type;

      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          b2BodyDef bodyDef;
          bodyDef.position.Set (width * 0.5, height * 0.5);
          space_actor->body = space_actor->world->CreateDynamicBody (&bodyDef);
          sync_body (space_actor);
          space_actor->body->SetMassFromShapes ();
        }
      else if (type == CLUTTER_BOX2D_STATIC)
        {
          b2BodyDef bodyDef;
          bodyDef.position.Set (0.0f, 0.0f);
          bodyDef.position.Set (width * 0.5, height * 0.5);
          space_actor->body = space_actor->world->CreateStaticBody (&bodyDef);
          sync_body (space_actor);
        }
    }
  else if (type == CLUTTER_BOX2D_META)
    {
      /* CLUTTER_BOX2D_META should not have a shape, but a body, to allow joints
        on it. */
    }
  g_hash_table_insert (CLUTTER_BOX2D_GET_PRIVATE (
                         space)->bodies, space_actor->body, space_actor);
}

static GObject *
stuff_constructor (GType                  type,
                   guint                  n_params,
                   GObjectConstructParam *params);
static void
clutter_box2d_dispose (GObject               *object);
static void
frame_cb (ClutterTimeline       *timeline,
          gint                   frame_num,
          gpointer               data);

static void
stuff_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (clutter_box2d_parent_class)->paint (actor);

  /* XXX: enable drawing the shapes over the actors as a debug-layer
   */
}


static void
clutter_box2d_class_init (ClutterBox2DClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class   = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->dispose     = clutter_box2d_dispose;
  gobject_class->constructor = stuff_constructor;
  actor_class->paint         = stuff_paint;

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DPrivate));
}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);
}



static GObject *
stuff_constructor (GType                  type,
                   guint                  n_params,
                   GObjectConstructParam *params)
{
  GObject              *object;
  ClutterBox2D         *self;
  ClutterBox2DPrivate  *priv;
  bool                  doSleep;

  b2AABB                worldAABB;

  worldAABB.lowerBound.Set (-500.0f, -1500.0f);
  worldAABB.upperBound.Set (1024.0f, 1500.0f);

  object = G_OBJECT_CLASS (clutter_box2d_parent_class)->constructor (
    type, n_params, params);

  self = CLUTTER_BOX2D (object);
  priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  priv->world = new b2World (worldAABB, /*gravity:*/ b2Vec2 (0.0f, 25.0f),
                             doSleep = false);

  priv->fps        = 40;
  priv->iterations = 20;

  priv->actors = g_hash_table_new (g_direct_hash, g_direct_equal);
  priv->bodies = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* we make the timeline play continously to have a constant source
   * of new-frame events as long as the timeline is playing.
   */
  priv->timeline = clutter_timeline_new ((int) (priv->fps * 10),
                                         (int) priv->fps);
  g_object_set (priv->timeline, "loop", TRUE, NULL);
  g_signal_connect (priv->timeline, "new-frame", G_CALLBACK (frame_cb), object);

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
}

static void
sync_body (ClutterBox2DActor *space_actor)
{
  gint w, h;
  gint x, y;
  gdouble rot;

  ClutterActor *actor = space_actor->actor;
  b2Body       *body  = space_actor->body;

  if (space_actor->type == CLUTTER_BOX2D_META)
    {
      x = clutter_actor_get_x (actor);
      y = clutter_actor_get_y (actor);
      body->SetXForm (b2Vec2 (x, y), 0.0);
      return;
    }

  ensure_shape (space_actor);

  if (!body)
    return;


  rot = clutter_actor_get_rotation (space_actor->actor,
                                    CLUTTER_Z_AXIS, NULL, NULL, NULL);

  w = clutter_actor_get_width (actor);
  h = clutter_actor_get_height (actor);

  x = clutter_actor_get_x (actor);
  y = clutter_actor_get_y (actor);

  SYNCLOG ("sync_body, actor was: %p %d,%d %dx%d\n", space_actor, x, y, w, h);

#define ANGLE_NEUTRAL_DAMPING    1.0 /* 0.8 */

  b2Vec2 position = body->GetPosition ();
  if (fabs (x - (position.x)) > 3.0 ||
      fabs (y - (position.y)) > 3.0 ||
      fabs (body->GetAngle()*(180/3.1415) - rot) > 3.0
      )
    {

      body->SetXForm (b2Vec2 (x,
                              y), (rot / (180 / 3.1415) * ANGLE_NEUTRAL_DAMPING));
      SYNCLOG ("\t setxform: %d, %d, %f\n", x, y, rot);
    }
}

static void
sync_actor (ClutterBox2DActor *space_actor)
{
  gint          w, h;
  ClutterActor *actor = space_actor->actor;
  b2Body       *body  = space_actor->body;

  if (!body)
    return;

  if (space_actor->type == CLUTTER_BOX2D_META)
    return;

  w = clutter_actor_get_width (actor);
  h = clutter_actor_get_height (actor);

  clutter_actor_set_positionu (actor,
                               CLUTTER_UNITS_FROM_FLOAT (body->GetPosition ().x),
                               CLUTTER_UNITS_FROM_FLOAT (body->GetPosition ().y));

  SYNCLOG ("setting' %f %f\n",
           body->GetPosition ().x,
           body->GetPosition ().y);

  clutter_actor_set_rotation (actor, CLUTTER_Z_AXIS,
                              body->GetAngle () * (180 / 3.1415),
                              0, 0, 0);
}


static void
clutter_box2d_iterate (ClutterBox2D *grspace,
                       guint         msecs)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (grspace);
  gint                 steps = priv->iterations;
  b2World             *world = priv->world;

  {
    GList *actors = g_hash_table_get_values (priv->actors);
    GList *iter;

    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *space_actor = (ClutterBox2DActor*) iter->data;
        sync_body (space_actor);
        space_actor->contacts = 0;
      }
    if (msecs == 0)
      return;

    world->Step (msecs / 1000.0, steps);

    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *space_actor = (ClutterBox2DActor*) iter->data;
        sync_actor (space_actor);
      }
    g_list_free (actors);
  }
}

static void
frame_cb (ClutterTimeline *timeline,
          gint             frame_num,
          gpointer         data)
{
  ClutterBox2D *grspace = CLUTTER_BOX2D (data);
  guint         msecs;

  clutter_timeline_get_delta (timeline, &msecs);

  clutter_box2d_iterate (grspace, msecs);
}

void
clutter_box2d_set_playing (ClutterBox2D  *space,
                           gboolean       playing)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (space);

  if (playing)
    {
      clutter_timeline_start (priv->timeline);
    }
  else
    {
      clutter_timeline_stop (priv->timeline);
    }
}

gboolean
clutter_box2d_get_playing (ClutterBox2D *space)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (space);

  return clutter_timeline_is_playing (priv->timeline);
}
