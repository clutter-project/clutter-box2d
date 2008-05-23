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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <clutter/clutter.h>
#include "clutter-box2d.h"
/*#include "tidy-cursor.h"*/

void tidy_cursor(gint x, gint y);

static gboolean clutter_box2d_actor_press   (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);
static gboolean clutter_box2d_actor_release (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);
static gboolean clutter_box2d_actor_motion  (ClutterActor *actor,
                                             ClutterEvent *event,
                                             gpointer      data);

void clutter_box2d_actor_set_manipulatable (ClutterActor *actor)
{
  clutter_actor_set_reactive (actor, TRUE);
  g_signal_connect (actor, "button-press-event",
                    G_CALLBACK (clutter_box2d_actor_press), NULL);
  g_signal_connect (actor, "motion-event",
                    G_CALLBACK (clutter_box2d_actor_motion), NULL);
  g_signal_connect (actor, "button-release-event",
                    G_CALLBACK (clutter_box2d_actor_release), NULL);
}


/* for multi-touch this needs to be made re-entrant */

static ClutterBox2DJoint *mouse_joint = NULL;
static ClutterActor      *manipulated_actor = NULL;
static ClutterUnit        start_x, start_y;

static void actor_died (gpointer data,
                        GObject *where_the_object_was)
{
  manipulated_actor = NULL;
}

static gboolean
clutter_box2d_actor_press (ClutterActor *actor,
                           ClutterEvent *event,
                           gpointer      data)
{

  if (clutter_box2d_get_simulating (
      CLUTTER_BOX2D (clutter_actor_get_parent (actor))))
    {

      start_x = CLUTTER_UNITS_FROM_INT (event->button.x);
      start_y = CLUTTER_UNITS_FROM_INT (event->button.y);

      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (actor),
        start_x, start_y,
        &start_x, &start_y);

      g_object_weak_ref (G_OBJECT (actor), actor_died, NULL);
      clutter_grab_pointer (actor);

      mouse_joint = clutter_box2d_add_mouse_joint (CLUTTER_BOX2D (
                        clutter_actor_get_parent (actor)),
                        actor, &(ClutterVertex){start_x, start_y});
      manipulated_actor = actor;
    }
  return FALSE;
}

static gboolean
clutter_box2d_actor_motion (ClutterActor *actor,
                          ClutterEvent *event,
                          gpointer      data)
{
  if (mouse_joint)
    {
      ClutterUnit x;
      ClutterUnit y;
      ClutterUnit dx;
      ClutterUnit dy;

      x = CLUTTER_UNITS_FROM_INT (event->motion.x);
      y = CLUTTER_UNITS_FROM_INT (event->motion.y);

      tidy_cursor (event->motion.x, event->motion.y);

      if (!manipulated_actor)
        return FALSE;
      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (actor),
        x, y,
        &x, &y);

       dx = x - start_x;
       dy = y - start_y;

       ClutterVertex target = { x, y };
       clutter_box2d_mouse_joint_update_target (mouse_joint, &target);
    }
  return FALSE;
}


static gboolean
clutter_box2d_actor_release (ClutterActor *actor,
                             ClutterEvent *event,
                             gpointer      data)
{


  if (mouse_joint)
    {
      clutter_box2d_joint_destroy (mouse_joint);
      mouse_joint = NULL;
      clutter_ungrab_pointer ();

      if (manipulated_actor)
        g_object_weak_unref (G_OBJECT (actor), actor_died, NULL);
      manipulated_actor = NULL;

      /* since the ungrab also was valid for this release we redeliver the
       * event to maintain the state of the click count.
       */
      if(1){ 
        ClutterEvent *synthetic_release;
        synthetic_release = clutter_event_new (CLUTTER_BUTTON_RELEASE);
        memcpy (synthetic_release, event, sizeof (ClutterButtonEvent));
        synthetic_release->any.source = NULL;
        clutter_do_event (synthetic_release); /* skip queue */
        clutter_event_free (synthetic_release);
      }

      return FALSE;
    }


  return FALSE;
}
