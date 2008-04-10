/* stuff - a content retrieval and management experiment.
 *
 * Copyright Øyvind Kolås <pippin@gimp.org> 2007-2008
 * Licensed under the GPL v3 or greater.
 */

#define FONT_SIZE    "15px"
#define PADDING      4

#include <clutter/clutter.h>
#include "util.h"
#include "popup.h"
#include "wrap-group.h"

static GList *actions = NULL;

void
popup_nuke (ClutterActor *stage,
            gint          x,
            gint          y)
{
  while (actions)
    {
      stuff_actor_fade_out_destroy (actions->data);
      /*clutter_actor_destroy (actions->data);*/
      actions = g_list_remove (actions, actions->data);
    }
  wrap_group_init (CLUTTER_GROUP (stage), x - 50, y - 50, 0, 10);
}

static gboolean
action_press (ClutterActor *action,
              ClutterEvent *event,
              gpointer      userdata)
{
  popup_nuke (clutter_stage_get_default (), 0, 0);
  return FALSE;
}

static gboolean
action_enter (ClutterActor *tag,
              ClutterEvent *event,
              gpointer      userdata)
{
  ClutterColor color = { 0xff, 0xff, 0x00, 0xff };

  clutter_label_set_color (CLUTTER_LABEL (userdata), &color);
  return FALSE;
}

static gboolean
action_leave (ClutterActor *tag,
              ClutterEvent *event,
              gpointer      userdata)
{
  ClutterColor color = { 0xff, 0xff, 0xff, 0xff };

  clutter_label_set_color (CLUTTER_LABEL (userdata), &color);
  return FALSE;
}


void
popup_add2 (const gchar *name,
            const gchar *visual,
            GCallback    callback,
            gpointer     user_data)
{
  ClutterColor  color  = { 0x00, 0x00, 0x00, 0x77 };
  ClutterColor  color2 = { 0xff, 0xff, 0xff, 0xff };

  ClutterActor *group;
  ClutterActor *label;
  ClutterActor *rectangle;
  ClutterActor *stage;

  stage = clutter_stage_get_default ();
  group = clutter_group_new ();

  rectangle = clutter_rectangle_new_with_color (&color);
  clutter_group_add (CLUTTER_GROUP (group), rectangle);
  label = clutter_label_new_with_text ("Sans " FONT_SIZE, name);

  clutter_group_add (CLUTTER_GROUP (group), label);

  actions = g_list_append (actions, group);
  wrap_group_add (CLUTTER_GROUP (stage), group);
  clutter_label_set_color (CLUTTER_LABEL (label), &color2);

  clutter_actor_set_opacity (group, 0x00);
  stuff_actor_fade_in (group, 0xff);

  clutter_actor_show_all (group);

  {
    gint w, h;
    w = clutter_actor_get_width (label);
    h = clutter_actor_get_height (label);
    clutter_actor_set_size (rectangle, w + 20, h + 10);
    clutter_actor_set_position (rectangle, 0, 0);
    clutter_actor_set_position (label, 2, 2);
  }

  clutter_actor_set_reactive (group, TRUE);

  if (callback)
    {
      g_signal_connect (group, "button-press-event",
                        callback, user_data);
    }
  g_signal_connect (group, "button-press-event",
                    (void*) action_press, user_data);

  g_signal_connect (group, "enter-event",
                    G_CALLBACK (action_enter), label);
  g_signal_connect (group, "leave-event",
                    G_CALLBACK (action_leave), label);

  g_object_set_data (G_OBJECT (group), "_", "foo");
  g_object_set_data (G_OBJECT (label), "_", "foo");
  g_object_set_data (G_OBJECT (rectangle), "_", "foo");
/*  return group;*/
}


void
popup_add (const gchar *name,
           const gchar *visual,
           GCallback    callback,
           gpointer     user_data)
{
  popup_add2 (name, visual, callback, user_data);
}


typedef struct
{
  void (*callback
        )(gpointer user_data,
          gdouble value);
  gfloat        min;
  gfloat        max;
  gfloat        initial;
  gpointer      userdata;
  ClutterActor *through;
  ClutterActor *self;
  ClutterActor *label;
  gboolean      in_drag;
} SliderData;



static gboolean
slider_motion (ClutterActor *slider,
               ClutterEvent *event,
               gpointer      userdata)
{
  SliderData *data = userdata;
  gint        ex, ey;

  clutter_event_get_coords (event, &ex, &ey);

  if (data->in_drag &&
      data->callback)
    {
      ClutterUnit xu2, yu2;

      if (clutter_actor_transform_stage_point (slider,
                                               CLUTTER_UNITS_FROM_DEVICE (ex),
                                               CLUTTER_UNITS_FROM_DEVICE (ey),
                                               &xu2, &yu2))
        {
          gint x, y;
          x = CLUTTER_UNITS_TO_DEVICE (xu2);
          y = CLUTTER_UNITS_TO_DEVICE (yu2);

          gfloat width = clutter_actor_get_width (slider);
          gfloat value = (x / width) * (data->max - data->min) + data->min;

          clutter_actor_set_position (data->through,
                                      x, 0);

          data->callback (data->userdata, value);
        }
    }

  /*popup_nuke (0, 0);*/
  return FALSE;
}

static gboolean
slider_press (ClutterActor *action,
              ClutterEvent *event,
              gpointer      userdata)
{
  SliderData *data = userdata;

  data->in_drag = TRUE;
  clutter_grab_pointer (action);
  return slider_motion (action, event, userdata);
}

static gboolean
slider_release (ClutterActor *action,
                ClutterEvent *event,
                gpointer      userdata)
{
  SliderData *data = userdata;

  data->in_drag = FALSE;
  clutter_ungrab_pointer ();
  return FALSE;
}


static gboolean
slider_enter (ClutterActor *tag,
              ClutterEvent *event,
              gpointer      userdata)
{
  SliderData  *data  = userdata;
  ClutterColor color = { 0xff, 0xff, 0x00, 0xff };

  clutter_label_set_color (CLUTTER_LABEL (data->label), &color);
  return FALSE;
}

static gboolean
slider_leave (ClutterActor *tag,
              ClutterEvent *event,
              gpointer      userdata)
{
  SliderData  *data  = userdata;
  ClutterColor color = { 0xff, 0xff, 0xff, 0xff };

  clutter_label_set_color (CLUTTER_LABEL (data->label), &color);
  return FALSE;
}


void
popup_add_slider (const gchar *name,
                  const gchar *visual,
                  gfloat       min,
                  gfloat       max,
                  gfloat       initial,
                  GCallback    callback,
                  gpointer     user_data)
{
  gint          width  = 140;
  ClutterColor  color  = { 0x33, 0x22, 0x11, 0x77 };
  ClutterColor  color2 = { 0xff, 0xff, 0xff, 0xff };
  ClutterColor  color3 = { 0xff, 0xff, 0x00, 0xff };

  ClutterActor *group;
  ClutterActor *label;
  ClutterActor *rectangle;
  ClutterActor *stage;
  ClutterActor *through;
  SliderData   *data;

  stage = clutter_stage_get_default ();
  group = clutter_group_new ();

  through   = clutter_rectangle_new_with_color (&color3);
  rectangle = clutter_rectangle_new_with_color (&color);
  clutter_group_add (CLUTTER_GROUP (group), rectangle);
  g_print ("slider: %s %s %p %p\n", name, visual, callback, user_data);
  label = clutter_label_new_with_text ("Sans " FONT_SIZE, name);

  clutter_group_add (CLUTTER_GROUP (group), label);

  actions = g_list_append (actions, group);
  wrap_group_add (CLUTTER_GROUP (stage), group);
  clutter_label_set_color (CLUTTER_LABEL (label), &color2);

  clutter_group_add (CLUTTER_GROUP (group), through);
  clutter_actor_set_size (through, 4, 30);

  clutter_actor_set_opacity (group, 0x00);
  stuff_actor_fade_in (group, 0xff);

  clutter_actor_show_all (group);
  {
    gint w, h;
    w = clutter_actor_get_width (label);
    h = clutter_actor_get_height (label);
    clutter_actor_set_size (rectangle, width, h + 10);
    clutter_actor_set_position (rectangle, 0, 0);
  }

  clutter_actor_set_position (through,
                              (initial - min) * width / (max - min), 0);

  clutter_actor_set_reactive (group, TRUE);

  data           = g_new0 (SliderData, 1);
  data->min      = min;
  data->max      = max;
  data->initial  = initial;
  data->callback = (void*) callback;
  data->userdata = user_data;
  data->through  = through;
  data->label    = label;

  /* make the data be collected, we don't really retrieve this */
  g_object_set_data_full (G_OBJECT (group), "slider-data", data, g_free);

  g_signal_connect (group, "button-press-event",
                    G_CALLBACK (slider_press), data);
  g_signal_connect (group, "button-release-event",
                    (void*) slider_release, data);
  g_signal_connect (group, "motion-event",
                    (void*) slider_motion, data);
  g_signal_connect (group, "enter-event",
                    G_CALLBACK (slider_enter), data);
  g_signal_connect (group, "leave-event",
                    G_CALLBACK (slider_leave), data);

  g_object_set_data (G_OBJECT (group), "_", "foo");
  g_object_set_data (G_OBJECT (label), "_", "foo");
  g_object_set_data (G_OBJECT (rectangle), "_", "foo");
  g_object_set_data (G_OBJECT (through), "_", "foo");
}

