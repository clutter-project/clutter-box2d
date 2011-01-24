/* clutter-box2d - Clutter box2d integration
 *
 * This file stores private data that should not be accessible by external
 * code.
 *
 * Copyright 2010 Intel Corporation
 * Licensed under the LGPL v2 or greater.
 */

#ifndef _CLUTTER_BOX2D_PRIVATE
#define _CLUTTER_BOX2D_PRIVATE

#include <glib.h>
#include <clutter/clutter.h>
#include <clutter-box2d/clutter-box2d.h>
#include "Box2D.h"

G_BEGIN_DECLS

struct _ClutterBox2DPrivate
{
  gint             iterations;  /* number of engine iterations per processing */
  gfloat           time_step;   /* Time step to simulate */
  gfloat           scale_factor; /* The scale factor of pixels to units */
  gfloat           inv_scale_factor; /* The inverse of the above */
  guint            iterate_id;  /* The iteration callback */

  b2World         *world;  /* The Box2D world which contains our simulation*/
  GHashTable      *actors; /* a hash table that maps actors to */
  GHashTable      *bodies; /* a hash table that maps bodies to */
  GHashTable      *joints;
  b2Body          *ground_body;
  gboolean         dirty;  /* Shapes need to be recreated */

  GList           *collisions; /* List of ClutterBox2DCollision contact 
                                * points from last iteration through time */
  ClutterBox2DContactListener *contact_listener;
};

struct _ClutterBox2DChildPrivate {
  /* Clutter-related variables */
  gboolean manipulatable;
  guint    press_handler;
  guint    captured_handler;
  gboolean was_reactive;

  gint               device_id;
  ClutterBox2DJoint *mouse_joint;
  gfloat        start_x, start_y;

  /* Box2D simulation-related variables */
  ClutterBox2DType  type; /* The type Static: the body affects collisions but
                             is not itself affected. Dynamic: the body is
                             affected by collisions. None: The object is not
                             included in the simulation. */
  gboolean          is_circle;
  ClutterVertex    *outline;
  b2Vec2           *b2outline;
  guint             n_vertices;

  b2Body           *body;   /* Box2D body, if any */
  b2Fixture        *fixture; /* Fixture for this body, if any */
  GList            *joints; /* list of joints this body participates in */
  b2World          *world;  /*the Box2D world (could be looked up through box2d)*/

  gfloat            density;
  gfloat            friction;
  gfloat            restitution;

  gfloat            old_x;   /* The last set position and rotation. */
  gfloat            old_y;   /* We store this to know when we need to resync */
  gdouble           old_rot; /* the box2d state with the Clutter state */
};

ClutterBox2DChild * clutter_box2d_get_child (ClutterBox2D *box2d,
                                             ClutterActor *actor);
void _clutter_box2d_sync_body (ClutterBox2D      *box2d,
                               ClutterBox2DChild *box2d_child);

G_END_DECLS

#endif
