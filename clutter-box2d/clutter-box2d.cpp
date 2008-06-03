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
#include "math.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (ClutterBox2D, clutter_box2d, CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                             clutter_container_iface_init));

void _clutter_box2d_sync_body (ClutterBox2DActor *box2d_actor);


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
  ClutterTimeline *timeline;    /* The timeline driving the simulation        */
};

typedef enum 
{
  CLUTTER_BOX2D_JOINT_DEAD,     /* An associated actor has been killed off */
  CLUTTER_BOX2D_JOINT_DISTANCE,
  CLUTTER_BOX2D_JOINT_PRISMATIC,
  CLUTTER_BOX2D_JOINT_REVOLUTE,
  CLUTTER_BOX2D_JOINT_MOUSE
} ClutterBox2DJointType;

ClutterBox2DActor *
clutter_box2d_get_actor (ClutterBox2D   *box2d,
                         ClutterActor   *actor)
{
  return CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta (CLUTTER_CONTAINER (box2d), actor));
}


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
        b2Vec2 b2gravity = b2Vec2(CLUTTER_UNITS_TO_FLOAT (gravity->x),
                                  CLUTTER_UNITS_TO_FLOAT (gravity->y));
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
                                                         "Simulatin",
                                                         "Whether ClutterBox2D is performing physical simulation or not.",
                                                         TRUE,
                                                         static_cast<GParamFlags>(G_PARAM_READWRITE)));

}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);
}

ClutterActor *   clutter_box2d_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CLUTTER_TYPE_BOX2D, NULL));
}

static GObject *
clutter_box2d_constructor (GType                  type,
                           guint                  n_params,
                           GObjectConstructParam *params)
{
  GObject              *object;
  ClutterBox2D         *self;
  ClutterBox2DPrivate  *priv;
  bool                  doSleep;

  b2AABB                worldAABB;

  /*  these magic numbers are the extent of the world,
   *  should be set large enough to contain most geometry,
   *  not sure how large this can be without impacting performance.
   */
  worldAABB.lowerBound.Set (-650.0f, -650.0f);
  worldAABB.upperBound.Set (650.0f, 650.0f);

  object = G_OBJECT_CLASS (clutter_box2d_parent_class)->constructor (
    type, n_params, params);

  self = CLUTTER_BOX2D (object);
  priv = CLUTTER_BOX2D_GET_PRIVATE (self);

  self->world = new b2World (worldAABB, /*gravity:*/ b2Vec2 (0.0f, 5.0f),
                             doSleep = false);

  priv->fps        = 25;
  priv->iterations = 50;

  self->actors = g_hash_table_new (g_direct_hash, g_direct_equal);
  self->bodies = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* we make the timeline play continously to have a constant source
   * of new-frame events as long as the timeline is playing.
   */
  priv->timeline = clutter_timeline_new ((int) (priv->fps * 10),
                                         (int) priv->fps);
  g_object_set (priv->timeline, "loop", TRUE, NULL);
  g_signal_connect (priv->timeline, "new-frame",
                    G_CALLBACK (clutter_box2d_iterate), object);

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
}


static void
clutter_box2d_create_child_meta (ClutterContainer *container,
                                 ClutterActor     *actor)

{
  ClutterChildMeta  *child_meta;
  ClutterBox2DActor *box2d_actor;

  child_meta = CLUTTER_CHILD_META (g_object_new (CLUTTER_TYPE_BOX2D_ACTOR, NULL));

  child_meta->container = container;
  child_meta->actor = actor;

  box2d_actor = CLUTTER_BOX2D_ACTOR (child_meta);
  box2d_actor->world = (b2World*)(CLUTTER_BOX2D (container)->world);

  g_hash_table_insert (CLUTTER_BOX2D (container)->actors, actor, child_meta);
}

static void
clutter_box2d_destroy_child_meta (ClutterContainer *box2d,
                                  ClutterActor     *actor)
{
  ClutterBox2DActor *box2d_actor =
     CLUTTER_BOX2D_ACTOR (clutter_container_get_child_meta ( box2d, actor));

  g_assert (box2d_actor->world);

  g_hash_table_remove (CLUTTER_BOX2D (box2d)->actors, actor);
  g_hash_table_remove (CLUTTER_BOX2D (box2d)->bodies, box2d_actor->body);
}

static ClutterChildMeta *
clutter_box2d_get_child_meta (ClutterContainer *box2d,
                ClutterActor     *actor)
{
  return CLUTTER_CHILD_META (g_hash_table_lookup (CLUTTER_BOX2D (box2d)->actors, actor));
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
      b2PolygonDef shapeDef;
      gint         width, height;
      gdouble      rot;

      width  = clutter_actor_get_width (CLUTTER_CHILD_META (box2d_actor)->actor);
      height = clutter_actor_get_height (CLUTTER_CHILD_META (box2d_actor)->actor);
      rot    = clutter_actor_get_rotation (CLUTTER_CHILD_META (box2d_actor)->actor,
                                           CLUTTER_Z_AXIS, NULL, NULL, NULL);


      shapeDef.SetAsBox (width * 0.5 * SCALE_FACTOR,
                         height * 0.5 * SCALE_FACTOR,
                         b2Vec2 (width * 0.5 * SCALE_FACTOR,
                         height * 0.5 * SCALE_FACTOR), 0);
      shapeDef.density   = 10.0f;
      shapeDef.friction = 0.2f;
      box2d_actor->shape = box2d_actor->body->CreateShape (&shapeDef);
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


  b2Vec2 position = body->GetPosition ();

  if (fabs (x * SCALE_FACTOR - (position.x)) > 0.1 ||
      fabs (y * SCALE_FACTOR - (position.y)) > 0.1 ||
      fabs (body->GetAngle()*(180/3.1415) - rot) > 2.0
      )
    {
      ensure_shape (box2d_actor);
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
  ClutterActor *actor = CLUTTER_CHILD_META (box2d_actor)->actor;
  b2Body       *body  = box2d_actor->body;

  if (!body)
    return;

  clutter_actor_set_positionu (actor,
       CLUTTER_UNITS_FROM_FLOAT (body->GetPosition ().x * INV_SCALE_FACTOR),
       CLUTTER_UNITS_FROM_FLOAT (body->GetPosition ().y * INV_SCALE_FACTOR));

  SYNCLOG ("setting actor position: ' %f %f angle: %f\n",
           body->GetPosition ().x,
           body->GetPosition ().y,
           body->GetAngle () * (180 / 3.1415));

  clutter_actor_set_rotation (actor, CLUTTER_Z_AXIS,
                              body->GetAngle () * (180 / 3.1415),
                              0, 0, 0);
}

static void
clutter_box2d_iterate (ClutterTimeline *timeline,
                       gint             frame_num,
                       gpointer         data)
{
  ClutterBox2D *box2d = CLUTTER_BOX2D (data);
  guint         msecs;

  clutter_timeline_get_delta (timeline, &msecs);
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
    world->Step (msecs / 1000.0, steps);


    /* syncronize actor to have geometrical sync with bodies */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *box2d_actor = (ClutterBox2DActor*) iter->data;
        _clutter_box2d_sync_actor (box2d_actor);
      }
    g_list_free (actors);
  }

}

void
clutter_box2d_set_simulating (ClutterBox2D  *box2d,
                              gboolean       simulating)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);

  if (simulating)
    {
      clutter_timeline_start (priv->timeline);
    }
  else
    {
      clutter_timeline_stop (priv->timeline);
    }
}

gboolean
clutter_box2d_get_simulating (ClutterBox2D *box2d)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);

  return clutter_timeline_is_playing (priv->timeline);
}

void *
clutter_box2d_actor_get_body (ClutterBox2D *box2d,
                              ClutterActor *actor)
{
  ClutterBox2DActor *box2d_actor = clutter_box2d_get_actor (box2d, actor);
  return box2d_actor->body;
}

void * clutter_box2d_get_world      (ClutterBox2D *box2d)
{
  return box2d->world;
}
