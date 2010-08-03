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

/**
 * ClutterBox2DChild:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _ClutterBox2DChild
{
  /*< private >*/
  ClutterChildMeta          parent_instance;
  ClutterBox2DChildPrivate *priv; /* not used anymore */
};

struct _ClutterBox2DChildClass
{
  /*< private >*/
  ClutterChildMetaClass parent_class;
};

GType   clutter_box2d_child_get_type  (void) G_GNUC_CONST;

void clutter_box2d_child_set_is_bullet (ClutterBox2D *box2d,
                                        ClutterActor *child,
                                        gboolean      is_bullet);
gboolean clutter_box2d_child_get_is_bullet (ClutterBox2D *box2d,
                                            ClutterActor *child);

void clutter_box2d_child_set_is_circle (ClutterBox2D *box2d,
                                        ClutterActor *child,
                                        gboolean      is_circle);
gboolean clutter_box2d_child_get_is_circle (ClutterBox2D *box2d,
                                            ClutterActor *child);

void clutter_box2d_child_set_outline (ClutterBox2D        *box2d,
                                      ClutterActor        *child,
                                      const ClutterVertex *outline,
                                      guint                n_vertices);
const ClutterVertex *clutter_box2d_child_get_outline (ClutterBox2D *box2d,
                                                      ClutterActor *child,
                                                      guint        *n_vertices);

void clutter_box2d_child_set_density (ClutterBox2D *box2d,
                                      ClutterActor *child,
                                      gfloat        density);
gfloat clutter_box2d_child_get_density (ClutterBox2D *box2d,
                                        ClutterActor *child);

void clutter_box2d_child_set_friction (ClutterBox2D *box2d,
                                       ClutterActor *child,
                                       gfloat        friction);
gfloat clutter_box2d_child_get_friction (ClutterBox2D *box2d,
                                         ClutterActor *child);

void clutter_box2d_child_set_restitution (ClutterBox2D *box2d,
                                          ClutterActor *child,
                                          gfloat        restitution);
gfloat clutter_box2d_child_get_restitution (ClutterBox2D *box2d,
                                            ClutterActor *child);

void clutter_box2d_child_set_linear_velocity (ClutterBox2D        *box2d,
                                              ClutterActor        *child,
                                              const ClutterVertex *velocity);
void clutter_box2d_child_get_linear_velocity (ClutterBox2D  *box2d,
                                              ClutterActor  *child,
                                              ClutterVertex *velocity);

void clutter_box2d_child_set_angular_velocity (ClutterBox2D  *box2d,
                                               ClutterActor  *child,
                                               gfloat        velocity);
gfloat clutter_box2d_child_get_angular_velocity (ClutterBox2D   *box2d,
                                                 ClutterActor   *child);

void clutter_box2d_child_set_mode (ClutterBox2D     *box2d,
                                   ClutterActor     *child,
                                   ClutterBox2DType  mode);
ClutterBox2DType clutter_box2d_child_get_mode (ClutterBox2D *box2d,
                                               ClutterActor *child);

void clutter_box2d_child_set_manipulatable (ClutterBox2D *box2d,
                                            ClutterActor *child,
                                            gboolean      manipulatable);
gboolean clutter_box2d_child_get_manipulatable (ClutterBox2D *box2d,
                                                ClutterActor *child);

G_END_DECLS

#endif
