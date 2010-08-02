/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DChild class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical
 * state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#ifndef _CLUTTER_BOX2D_CHILD_H
#define _CLUTTER_BOX2D_CHILD_H

#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "math.h"

G_BEGIN_DECLS

#define CLUTTER_TYPE_BOX2D_CHILD    clutter_box2d_child_get_type ()

#define CLUTTER_BOX2D_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                               CLUTTER_TYPE_BOX2D_CHILD, ClutterBox2DChild))

#define CLUTTER_BOX2D_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
                            CLUTTER_TYPE_BOX2D_CHILD, ClutterBox2DChildClass))

#define CLUTTER_IS_BOX2D_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                               CLUTTER_TYPE_BOX2D_CHILD))

#define CLUTTER_IS_BOX2D_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                            CLUTTER_TYPE_BOX2D_CHILD))

#define CLUTTER_BOX2D_CHILD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                              CLUTTER_TYPE_BOX2D_CHILD, ClutterBox2DChildClass))

typedef struct _ClutterBox2DChild        ClutterBox2DChild;
typedef struct _ClutterBox2DChildPrivate ClutterBox2DChildPrivate;
typedef struct _ClutterBox2DChildClass   ClutterBox2DChildClass;

struct _ClutterBox2DChild
{
  /*< private >*/
  ClutterChildMeta          parent_instance;
  ClutterBox2DChildPrivate *priv; /* not used anymore */

  ClutterBox2DType  type; /* The type Static: the body affects collisions but
                             is not itself affected. Dynamic: the body is
                             affected by collisions.*/ 
  gboolean          is_circle;
  ClutterVertex    *outline;

  b2Body           *body;   /* Box2D body, if any */
  b2Shape          *shape;  /* shape attached to this body, if any */
  GList            *joints; /* list of joints this body participates in */
  b2World          *world;  /*the Box2D world (could be looked up through box2d)*/

  gfloat            density;
  gfloat            friction;
  gfloat            restitution;
};

struct _ClutterBox2DChildClass
{
  /*< private >*/
  ClutterChildMetaClass parent_class;
};

ClutterBox2DChild * clutter_box2d_get_child (ClutterBox2D   *box2d,
                                             ClutterActor   *actor);

GType   clutter_box2d_child_get_type  (void) G_GNUC_CONST;

G_END_DECLS

#endif
