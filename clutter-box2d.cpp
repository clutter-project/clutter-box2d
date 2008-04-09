/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of Box2D
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

enum
{
  PROP_0,
  PROP_GRAVITY
};

struct _ClutterBox2DPrivate
{
  b2World         *world;      /* The Box2D world which contains our simulation */
  GHashTable      *actors;     /* a hash table that maps actors to bodies */
  GHashTable      *bodies;     /* a hash table that maps bodies to actors */

  gdouble          fps;        /* The framerate simulation is running at */
  gint             iterations; /* number of engine iterations per processing */

  ClutterTimeline *timeline;   /* The timeline driving the simulation */
};


/* This structure contains the combined state of an actor and a body,
 * */
typedef struct _ClutterBox2DActor ClutterBox2DActor;
struct _ClutterBox2DActor
{
  ClutterBox2DType  type;           /* The type
                                        Static: the body affects collisions but
                                          is not itself moved.
                                        Dynamic: the body is affected by collisi                                          s and moves.*/
                                                
  ClutterBox2D     *clutter_box2d;
  ClutterActor     *actor;

  b2Body           *body;
  b2World          *world;
  b2Shape          *shape;

  gdouble scale_x;  /* cached values of geometry, kept to */
  gdouble scale_y;  /* see if the shape needs recomputation */
  gint    width;
  gint    height;
  gint    contacts;
};

typedef enum 
{
  CLUTTER_BOX2D_JOINT_DEAD, /* An associated body has been killed off */
  CLUTTER_BOX2D_JOINT_DISTANCE,
  CLUTTER_BOX2D_JOINT_PRISMATIC,
  CLUTTER_BOX2D_JOINT_REVOLUTE,
  CLUTTER_BOX2D_JOINT_MOUSE,
  CLUTTER_BOX2D_JOINT_PULLEY,
  CLUTTER_BOX2D_JOINT_GEAR,
} ClutterBox2DJointType;


/* A Box2DJoint contains all relevant tracking information
 * for a box2d joint.
 */
struct _ClutterBox2DJoint
{
  ClutterBox2D          *box2d;
  ClutterBox2DJointType *type;
  b2Joint               *joint;
  ClutterBox2DActor     *actor1;  /* */
  ClutterBox2DActor     *actor2;  /* */

  ClutterVertex          local_anchor1; /* target */
  ClutterVertex          local_anchor2;
  ClutterVertex          local_axis;

  ClutterUnit            length;
  ClutterUnit            frequency;
  ClutterUnit            damping_ratio;

  ClutterUnit            reference_angle;


};

struct _ClutterBox2DSpring
{
  ClutterBox2D      *clutter_box2d;
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


static void sync_body  (ClutterBox2DActor *space_actor);
static void sync_actor (ClutterBox2DActor *space_actor);


static GObject * clutter_box2d_constructor (GType                  type,
                                            guint                  n_params,
                                            GObjectConstructParam *params);
static void      clutter_box2d_dispose     (GObject               *object);

static void      clutter_box2d_iterate     (ClutterTimeline       *timeline,
                                            gint                   frame_num,
                                            gpointer               data);

static void
clutter_box2d_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (clutter_box2d_parent_class)->paint (actor);

  /* XXX: enable drawing the shapes over the actors as a debug-layer
   */
}

void
clutter_box2d_set_gravity (ClutterBox2D        *box2d,
                           const ClutterVertex *gravity)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2Vec2 b2gravity = b2Vec2(CLUTTER_UNITS_TO_FLOAT (gravity->x),
                     CLUTTER_UNITS_TO_FLOAT (gravity->y));
     
  priv->world->SetGravity (b2gravity); 
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
        clutter_box2d_set_gravity (box2d, (ClutterVertex*)g_value_get_boxed (value));
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_box2d_class_init (ClutterBox2DClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class   = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->dispose     = clutter_box2d_dispose;
  gobject_class->constructor = clutter_box2d_constructor;
  gobject_class->set_property = clutter_box2d_set_property;
  actor_class->paint         = clutter_box2d_paint;

  g_type_class_add_private (gobject_class, sizeof (ClutterBox2DPrivate));

  g_object_class_install_property (gobject_class,
                                   PROP_GRAVITY,
                                   g_param_spec_boxed ("gravity",
                                                       "Gravity",
                                                       "",
                                                       CLUTTER_TYPE_VERTEX,
                                                       G_PARAM_WRITABLE));


}

static void
clutter_box2d_init (ClutterBox2D *self)
{
  self->priv = CLUTTER_BOX2D_GET_PRIVATE (self);
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
  worldAABB.lowerBound.Set (-6500.0f, -6500.0f);
  worldAABB.upperBound.Set (6500.0f, 6500.0f);

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


/* make sure that the shape attached to the body matches the clutter realms
 * idea of the shape.
 */
static inline void
ensure_shape (ClutterBox2DActor *space_actor)
{
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
      shapeDef.density   = 10.0f;
      shapeDef.friction = 0.2f;
      space_actor->shape = space_actor->body->CreateShape (&shapeDef);
    }
  else
    {
      /*XXX: recreate shape on every ensure? */
    }
}

/* Set the type of physical object an actor in a Box2D group is of.
 */
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
      b2BodyDef bodyDef;

      SYNCLOG ("making an actor to be %s\n",
               type == CLUTTER_BOX2D_STATIC ? "static" : "dynamic");
      
      space_actor->type = type;

      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
          space_actor->body = space_actor->world->CreateDynamicBody (&bodyDef);
        }
      else if (type == CLUTTER_BOX2D_STATIC)
        {
          space_actor->body = space_actor->world->CreateStaticBody (&bodyDef);
        }
      sync_body (space_actor);
      space_actor->body->SetMassFromShapes ();
    }
  else if (type == CLUTTER_BOX2D_META)
    {
      SYNCLOG ("making an actor to be meta\n");
      /* CLUTTER_BOX2D_META has a body but not a shape, this allows
       * attaching joints and springs.
       */
    }
  g_hash_table_insert (CLUTTER_BOX2D_GET_PRIVATE (
                         space)->bodies, space_actor->body, space_actor);
}

/* Synchronise the state of the Box2D body with the
 * current geomery of the actor, only really do it if
 * we differ more than a certain delta to avoid disturbing
 * the physics computation
 */
static void
sync_body (ClutterBox2DActor *space_actor)
{
  gint w, h;
  gint x, y;
  gdouble rot;

  ClutterActor *actor = space_actor->actor;
  b2Body       *body  = space_actor->body;

  /*
   * If the type is meta we closely track the actor's geometry
   */
  if (space_actor->type == CLUTTER_BOX2D_META)
    {
      x = clutter_actor_get_x (actor);
      y = clutter_actor_get_y (actor);
      body->SetXForm (b2Vec2 (x, y), 0.0);
      return;
    }

  if (!body)
    return;

  rot = clutter_actor_get_rotation (space_actor->actor,
                                    CLUTTER_Z_AXIS, NULL, NULL, NULL);

  w = clutter_actor_get_width (actor);
  h = clutter_actor_get_height (actor);

  x = clutter_actor_get_x (actor);
  y = clutter_actor_get_y (actor);

  SYNCLOG ("sync_body, actor was: %p %d,%d %dx%d\n", space_actor, x, y, w, h);

  b2Vec2 position = body->GetPosition ();
  if (fabs (x - (position.x)) > 3.0 ||
      fabs (y - (position.y)) > 3.0 ||
      fabs (body->GetAngle()*(180/3.1415) - rot) > 3.0
      )
    {
      ensure_shape (space_actor);
      body->SetXForm (b2Vec2 (x, y), rot / (180 / 3.1415));

      SYNCLOG ("\t setxform: %d, %d, %f\n", x, y, rot);
    }
}

/* Synchronise actor geometry from body, rounding erorrs introduced here should
 * be smaller than the threshold accepted when sync_body is being run to avoid
 * introducing errors.
 */
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
  b2World             *world = priv->world;

  {
    GList *actors = g_hash_table_get_values (priv->actors);
    GList *iter;

    /* First we check for each actor the need for, and perform a sync
     * from the actor to the body before running simulation
     */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *space_actor = (ClutterBox2DActor*) iter->data;
        sync_body (space_actor);
        space_actor->contacts = 0;
      }

    if (msecs == 0)
      return;

    /* Iterate Box2D simulation of bodies */
    world->Step (msecs / 1000.0, steps);


    /* syncronize actor to have geometrical sync with bodies */
    for (iter = actors; iter; iter = g_list_next (iter))
      {
        ClutterBox2DActor *space_actor = (ClutterBox2DActor*) iter->data;
        sync_actor (space_actor);
      }
    g_list_free (actors);
  }
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

void *
clutter_box2d_actor_get_body (ClutterBox2D *space,
                              ClutterActor *actor)
{
  ClutterBox2DActor *space_actor = clutter_box2d_get_actor (space, actor);
  return space_actor->body;
}

ClutterBox2DType clutter_box2d_actor_apply_force (ClutterBox2D     *box2d,
                                                  ClutterActor     *actor,
                                                  ClutterVertex    *force,
                                                  ClutterVertex    *position)
{
  ClutterBox2DActor *space_actor = clutter_box2d_get_actor (box2d, actor);
  b2Vec2 b2force (CLUTTER_UNITS_TO_FLOAT (force->x),
                  CLUTTER_UNITS_TO_FLOAT (force->y));
  b2Vec2 b2position (CLUTTER_UNITS_TO_FLOAT (position->x),
                     CLUTTER_UNITS_TO_FLOAT (position->y));

  space_actor->body->ApplyForce (
           space_actor->body->GetWorldVector(b2force),
           space_actor->body->GetWorldVector(b2position));
}

void * clutter_box2d_get_world      (ClutterBox2D *space)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (space);
  return priv->world;
}

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
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2RevoluteJointDef jd;
  /*b2DistanceJointDef jd;*/
  /*b2PrismaticJointDef jd;*/
  b2Vec2 anchor (x_a, y_a);

  /*jd.body1 = clutter_box2d_get_actor (box2d, actor_a)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor_b)->body;*/
  /*jd.localAnchor1.Set(x_a, y_a);
  jd.localAnchor2.Set(x_b, y_b);*/
  /*jd.length = min_len;
  jd.lowerTranslation = min_len;
  jd.upperTranslation = max_len;*/
  /*jd.enableLimit = true;*/
  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_actor (box2d, actor_a)->body,
                clutter_box2d_get_actor (box2d, actor_b)->body,
                anchor);
  priv->world->CreateJoint (&jd);

  return NULL;
}

static ClutterBox2DJoint *joint_new (ClutterBox2D *box2d,
                                     b2Joint      *joint)
{
   ClutterBox2DJoint *self = g_new0 (ClutterBox2DJoint, 1);
   self->box2d = box2d;
   self->joint = joint;
   return self;
}


ClutterBox2DJoint *clutter_box2d_add_distance_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              length,
                                                     gdouble              frequency,
                                                     gdouble              damping_ratio)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2DistanceJointDef jd;

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y));
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y));
  jd.length = length;
  jd.frequencyHz = 0.0;
  jd.dampingRatio = 0.0;

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}


ClutterBox2DJoint *clutter_box2d_add_distance_joint2 (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor1,
                                                      const ClutterVertex *anchor2,
                                                      gdouble              frequency,
                                                      gdouble              damping_ratio)
{
  /* this one should compute the length automatically based on the
   * initial configuration?
   */
  return NULL;
}


ClutterBox2DJoint *clutter_box2d_add_revolute_joint (ClutterBox2D        *box2d,
                                                     ClutterActor        *actor1,
                                                     ClutterActor        *actor2,
                                                     const ClutterVertex *anchor1,
                                                     const ClutterVertex *anchor2,
                                                     gdouble              reference_angle)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2RevoluteJointDef jd;

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y));
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y));
  jd.referenceAngle = reference_angle;

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *clutter_box2d_add_revolute_joint2 (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2RevoluteJointDef jd;
  b2Vec2 ancho  (CLUTTER_UNITS_TO_FLOAT (anchor->x),
                 CLUTTER_UNITS_TO_FLOAT (anchor->y));

  jd.collideConnected = false;
  jd.Initialize(clutter_box2d_get_actor (box2d, actor1)->body,
                clutter_box2d_get_actor (box2d, actor2)->body,
                ancho);
  return joint_new (box2d, priv->world->CreateJoint (&jd));
}

ClutterBox2DJoint *clutter_box2d_add_prismatic_joint (ClutterBox2D        *box2d,
                                                      ClutterActor        *actor1,
                                                      ClutterActor        *actor2,
                                                      const ClutterVertex *anchor1,
                                                      const ClutterVertex *anchor2,
                                                      gdouble              min_length,
                                                      gdouble              max_length,
                                                      const ClutterVertex *axis)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2PrismaticJointDef jd;

  jd.collideConnected = false;
  jd.body1 = clutter_box2d_get_actor (box2d, actor1)->body;
  jd.body2 = clutter_box2d_get_actor (box2d, actor2)->body;
  jd.localAnchor1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor1->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor1->y));
  jd.localAnchor2 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (anchor2->x),
                           CLUTTER_UNITS_TO_FLOAT (anchor2->y));
  jd.lowerTranslation = min_length;
  jd.lowerTranslation = max_length;
  jd.enableLimit = true;
  jd.localAxis1 = b2Vec2(CLUTTER_UNITS_TO_FLOAT (axis->x),
                         CLUTTER_UNITS_TO_FLOAT (axis->y));

  return joint_new (box2d, priv->world->CreateJoint (&jd));
}


ClutterBox2DJoint *clutter_box2d_add_mouse_joint    (ClutterBox2D     *box2d,
                                                     ClutterActor     *actor,
                                                     ClutterVertex    *target)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (box2d);
  b2MouseJointDef md;

  md.body1 = priv->world->GetGroundBody();
  md.body2 = clutter_box2d_get_actor (box2d, actor)->body;
  md.target = b2Vec2(CLUTTER_UNITS_TO_FLOAT (target->x),
                     CLUTTER_UNITS_TO_FLOAT (target->y));
  md.body1->WakeUp ();
  md.maxForce = 1000.0f * md.body2->GetMass ();

  return joint_new (box2d, priv->world->CreateJoint(&md));
}

void clutter_box2d_mouse_joint_update_target (ClutterBox2DJoint   *joint,
                                              const ClutterVertex *target)
{
  b2Vec2 b2target = b2Vec2(CLUTTER_UNITS_TO_FLOAT (target->x),
                           CLUTTER_UNITS_TO_FLOAT (target->y));
  /* FIXME: use a proper ClutterBox2DJoint structure to avoid this
   *        ugly cast
   */
  static_cast<b2MouseJoint*>(joint->joint)->SetTarget(b2target);
}

/* FIXME: use a proper ClutterBox2DJoint */
void clutter_box2d_joint_remove (ClutterBox2DJoint   *joint)
{
  ClutterBox2DPrivate *priv = CLUTTER_BOX2D_GET_PRIVATE (joint->box2d);
  priv->world->DestroyJoint (joint->joint);
  g_free (joint);
  /* XXX: and remove from list */
}
