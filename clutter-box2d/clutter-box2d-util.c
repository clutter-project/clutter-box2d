/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DActor class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */

#include <clutter/clutter.h>
#include "clutter-box2d-util.h"

typedef struct TrackData
{
  ClutterActor *self;

  ClutterActor *other;
  gfloat   rel_x;
  gfloat   rel_y;

  gdouble       rel_angle;
  gdouble       prev_angle;

  gint          x_handler;
  gint          y_handler;

  gint          rotation_handler;
} TrackData;


static void clutter_box2d_actor_track_position (ClutterActor *actor,
                                                GParamSpec   *pspec,
                                                gpointer      data)
{
  TrackData *td = data;
  gfloat x, y;

  clutter_actor_get_position (td->other, &x, &y);
  clutter_actor_set_position (td->self, x + td->rel_x, y + td->rel_y);

  clutter_actor_queue_redraw (actor);
}

static void clutter_box2d_actor_track_rotation (ClutterActor *actor,
                                                GParamSpec   *pspec,
                                                gpointer      data)
{
  TrackData *td = data;
  gdouble angle;

  angle = clutter_actor_get_rotation (td->other, CLUTTER_Z_AXIS, 0,0,0);
  if (angle != td->prev_angle)
    {
      clutter_actor_set_rotation (td->self, CLUTTER_Z_AXIS,
                                   angle + td->rel_angle,
                                   0, 0, 0);
      td->prev_angle = angle;
    }
}

/* 
 * Make an actor maintain the relative position to another actor, the position
 * of actor will change when notify events are emitted for the "x" or "y"
 * properties on other.
 *
 * Neither of the actors have to be children of a box2d group, but this is probably
 * most useful when "other" is a box2d controlled actor (that might be hidden) and
 * actor is a user visible ClutterActor.
 */

void clutter_box2d_actor_track (ClutterActor           *actor,
                                ClutterActor           *other,
                                ClutterBox2DTrackFlags  flags)
{
  TrackData *td;
  td = g_object_get_data (G_OBJECT (actor), "track-data");
  if (!td)
    {
      td = g_new0 (TrackData, 1);
      g_object_set_data (G_OBJECT (actor), "track-data", td);
      td->self = actor;
    }

  if (td->x_handler)
    {
      g_signal_handler_disconnect (td->other, td->x_handler);
      td->x_handler = 0;
    }
  if (td->y_handler)
    {
      g_signal_handler_disconnect (td->other, td->y_handler);
      td->y_handler = 0;
    }
  if (td->rotation_handler)
    {
      g_signal_handler_disconnect (td->other, td->rotation_handler);
      td->rotation_handler = 0;
    }
  if (!other)
    {
      return;
    }
  td->other = other;


  td->rel_x = clutter_actor_get_x (actor) - clutter_actor_get_x (other);
  td->rel_y = clutter_actor_get_y (actor) - clutter_actor_get_y (other);

  td->rel_angle = clutter_actor_get_rotation (actor, CLUTTER_Z_AXIS, 0, 0, 0)
                - clutter_actor_get_rotation (other, CLUTTER_Z_AXIS, 0, 0, 0);

  /* listen for notifies when the others position change and then change
   * the position of ourself accordingly.
   */
  if (flags & CLUTTER_BOX2D_TRACK_POSITION)
    {
#if 0
      td->x_handler = g_signal_connect (G_OBJECT (other), "notify::x",
                                      G_CALLBACK (clutter_box2d_actor_track_position), td);
      td->y_handler = g_signal_connect (G_OBJECT (other), "notify::y",
                                      G_CALLBACK (clutter_box2d_actor_track_position), td);
#else
      /* we listen to notifications of changes on the allocation detail not the individual x and y
       * properties (doing so seems to be slightly broken)
       */
      td->y_handler = g_signal_connect (G_OBJECT (other), "notify::allocation",
                                      G_CALLBACK (clutter_box2d_actor_track_position), td);
#endif
    }

  if (flags & CLUTTER_BOX2D_TRACK_ROTATION)
    {
      td->rotation_handler = g_signal_connect (G_OBJECT (other), "notify::rotation-angle-z",
                             G_CALLBACK (clutter_box2d_actor_track_rotation), td);
    }
}
