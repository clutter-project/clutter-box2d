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
  guint             n_vertices;

  b2Body           *body;   /* Box2D body, if any */
  b2Shape          *shape;  /* shape attached to this body, if any */
  GList            *joints; /* list of joints this body participates in */
  b2World          *world;  /*the Box2D world (could be looked up through box2d)*/

  gfloat            density;
  gfloat            friction;
  gfloat            restitution;
};

ClutterBox2DChild * clutter_box2d_get_child (ClutterBox2D *box2d,
                                             ClutterActor *actor);

G_END_DECLS

#endif
