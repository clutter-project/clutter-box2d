#include <clutter/clutter.h>

static gboolean
label_action_press (ClutterActor *actor,
                    ClutterEvent *event,
                    gpointer      data)
{
  void (*action
        )(ClutterActor * label,
          gpointer userdata) = data;
  gpointer user_data = g_object_get_data (G_OBJECT (actor), "la-ud");

  if (action)
    action (actor, user_data);
  else
    g_print ("no action\n");

  return FALSE;
}

static gboolean
label_action_enter (ClutterActor *actor,
                    ClutterEvent *event,
                    gpointer      data)
{
  clutter_actor_set_opacity (actor, 0xff);
  return FALSE;
}

static gboolean
label_action_leave (ClutterActor *actor,
                    ClutterEvent *event,
                    gpointer      data)
{
  clutter_actor_set_opacity (actor, 0x77);
  return FALSE;
}

ClutterActor *
label_action (const gchar            *font,
              const gchar            *label,
              const gchar            *color,
              void                    (*action)(ClutterActor *label,
                                                gpointer userdata),
              gpointer                userdata
              )
{
  ClutterActor *actor;
  ClutterColor  ccol;

  clutter_color_from_string (&ccol, color);
  actor = clutter_text_new_full (font, label, &ccol);
  clutter_actor_set_reactive (actor, TRUE);
  clutter_actor_set_opacity (actor, 0x77);

  g_signal_connect (actor, "button-press-event",
                    G_CALLBACK (label_action_press), action);
  g_signal_connect (actor, "enter-event",
                    G_CALLBACK (label_action_enter), action);
  g_signal_connect (actor, "leave-event",
                    G_CALLBACK (label_action_leave), action);
  g_object_set_data (G_OBJECT (actor), "la-ud", userdata);

  return actor;
}
