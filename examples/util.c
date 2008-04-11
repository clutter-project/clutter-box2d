/* stuff - a content retrieval and management experiment.
 *
 * Copyright Øyvind Kolås <pippin@gimp.org> 2007-2008
 * Licensed under the GPL v3 or greater.
 */

#include <glib.h>
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

typedef struct
{
  void (*func
        )(void*);
  gpointer data;
} DeferredCall;

static gboolean
deferred_call_invoker (gpointer data)
{
  DeferredCall *call_data;

  call_data = data;
  call_data->func (call_data->data);
  return FALSE;
}

void
stuff_deferred_call (void     (*func)(void*) ,
                     gpointer data)
{
  DeferredCall *call_data;

  call_data = g_malloc (sizeof (DeferredCall));

  call_data->func = func;
  call_data->data = data;

  g_idle_add_full (G_PRIORITY_HIGH,
                   deferred_call_invoker,
                   call_data,
                   g_free);
}

static ClutterEffectTemplate *
fade_template (void)
{
  static ClutterEffectTemplate *template = NULL;

  if (template == NULL)
    {
      template = clutter_effect_template_new (
        clutter_timeline_new_for_duration (400),
        CLUTTER_ALPHA_RAMP_INC);
    }
  return template;
}

static void
fade_out_complete (ClutterActor *actor,
                   gpointer      data)
{
  if (g_object_get_data (G_OBJECT (actor), "fade-in") != (gpointer) 0x1)
    {
      clutter_actor_set_opacity (actor, GPOINTER_TO_UINT (data));
      clutter_actor_hide (actor);
    }
}

static void
fade_in_complete (ClutterActor *actor,
                  gpointer      data)
{
  if (g_object_get_data (G_OBJECT (actor), "fade-in") == (gpointer) 0x1)
    {
      clutter_actor_set_opacity (actor, GPOINTER_TO_UINT (data));
      clutter_actor_show (actor);
      g_object_set_data (G_OBJECT (actor), "fade-in", 0x0);
    }
}

static void
fade_out_destroy_complete (ClutterActor *actor,
                           gpointer      data)
{
  clutter_actor_set_opacity (actor, GPOINTER_TO_UINT (data));
  clutter_actor_destroy (actor);
}

void
stuff_actor_fade_out (ClutterActor *actor)
{
  g_object_set_data (G_OBJECT (actor), "fade-in", (gpointer) 0x0);
  clutter_effect_fade (fade_template (),
                       actor,
                       0,
                       fade_out_complete,
                       GUINT_TO_POINTER ((guint) clutter_actor_get_opacity (
                                           actor)));
}


void
stuff_actor_fade_out_destroy (ClutterActor *actor)
{
  clutter_effect_fade (fade_template (),
                       actor,
                       0,
                       fade_out_destroy_complete, NULL);
}

void
stuff_actor_fade_in (ClutterActor *actor,
                     gint          opacity)
{
  clutter_actor_show (actor);
  g_object_set_data (G_OBJECT (actor), "fade-in", (gpointer) 0x1);
  clutter_effect_fade (fade_template (),
                       actor,
                       opacity,
                       fade_in_complete, GUINT_TO_POINTER ((guint) opacity));
}


/****************************************************************/

ClutterActor *
add_hand (ClutterActor *group,
          gint          x,
          gint          y)
{
  ClutterActor     *actor;
  static GdkPixbuf *hand_pixbuf = NULL;

  if (!hand_pixbuf)
    {
      GError *error = NULL;
      hand_pixbuf = gdk_pixbuf_new_from_file (ASSETS_DIR "redhand.png", &error);
      if (error)
        g_error ("Unable to load redhand.png: %s", error->message);
    }

  actor = clutter_texture_new_from_pixbuf (hand_pixbuf);
  clutter_group_add (CLUTTER_GROUP (group), actor);

  clutter_actor_set_opacity (actor, 1.0 * 255);
  clutter_actor_set_position (actor, x, y);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), actor, CLUTTER_BOX2D_DYNAMIC);
  clutter_actor_show (actor);
  return actor;
}

static void
add_static_box (ClutterActor *group,
                gint          x,
                gint          y,
                gint          width,
                gint          height)
{
  ClutterActor *box;
  box = clutter_rectangle_new ();
  clutter_actor_set_size (box, width, height);
  clutter_actor_set_position (box, x, y);
  clutter_group_add (CLUTTER_GROUP (group), box);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), box, CLUTTER_BOX2D_STATIC);
  clutter_actor_show (box);
}

void
add_cage (ClutterActor *group,
          gboolean      roof)
{
  ClutterActor *stage;
  gint width, height;

  stage = clutter_stage_get_default ();
  width = clutter_actor_get_width (stage);
  height = clutter_actor_get_height (stage);

  if (roof)
    {
      add_static_box (group, -100, -100, width + 200, 100);
    }
  add_static_box (group, -100, height, width + 200, 100);

  add_static_box (group, -100, -height, 100, height * 2);
  add_static_box (group, width, -height, 100, height * 2);
}
