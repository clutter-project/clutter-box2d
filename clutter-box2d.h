/*
 * This file implements a special ClutterGroup subclass that
 * allows simulating physical interactions of it's child actors
 * through the use of box2d
 *
 * Copyright 2007 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 */

#ifndef _CLUTTER_BOX2D_H
#define _CLUTTER_BOX2D_H

#include <clutter/clutter-group.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_BOX2D    clutter_box2d_get_type ()

#define CLUTTER_BOX2D(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                               CLUTTER_TYPE_BOX2D, ClutterBox2D))

#define CLUTTER_BOX2D_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
                            CLUTTER_TYPE_BOX2D, ClutterBox2DClass))

#define CLUTTER_IS_BOX2D(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                               CLUTTER_TYPE_BOX2D))

#define CLUTTER_IS_BOX2D_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                            CLUTTER_TYPE_BOX2D))

#define CLUTTER_BOX2D_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                              CLUTTER_TYPE_BOX2D, ClutterBox2DClass))

typedef struct _ClutterBox2D        ClutterBox2D;
typedef struct _ClutterBox2DClass   ClutterBox2DClass;
typedef struct _ClutterBox2DPrivate ClutterBox2DPrivate;

struct _ClutterBox2D
{
  ClutterGroup         parent_instance;
  ClutterBox2DPrivate *priv;
};

struct _ClutterBox2DClass
{
  /*< private >*/
  ClutterGroupClass parent_class;
};

typedef enum 
{
  CLUTTER_BOX2D_UNINITIALIZED = 0,
  CLUTTER_BOX2D_DYNAMIC,
  CLUTTER_BOX2D_STATIC,
  CLUTTER_BOX2D_META
} ClutterBox2DType;

GType            clutter_box2d_get_type         (void) G_GNUC_CONST;
void             clutter_box2d_set_playing      (ClutterBox2D  *space,
                                                 gboolean       playing);
gboolean         clutter_box2d_get_playing      (ClutterBox2D  *space);

void             clutter_box2d_actor_set_type   (ClutterBox2D     *space,
                                                 ClutterActor     *actor,
                                                 ClutterBox2DType  type);

ClutterBox2DType clutter_box2d_actor_get_type   (ClutterBox2D     *space,
                                                 ClutterActor     *actor);

G_END_DECLS

#endif
