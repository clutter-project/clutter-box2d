#include <clutter/clutter.h>
#include "scenes.h"

static GList *scenes = NULL;

void
scenes_add_scene (const gchar *title,
                  void         (*create)(Scene*scene))
{
  Scene *scene = g_new0 (Scene, 1);

  scene->title  = g_strdup (title);
  scene->create = create;
  scenes        = g_list_append (scenes, scene);
}


gint     current_scene = -1;
gboolean simulating       = TRUE;

Scene *
get_scene_no (gint no)
{
  return (Scene*) (g_list_nth (scenes, no)->data);
}

ClutterActor *
scene_get_group (void)
{
  return get_scene_no (current_scene)->group;
}

void
scene_activate (gint scene_no)
{
  if (scene_no < 0)
    scene_no = g_list_length (scenes) + scene_no;

  g_print ("go to scene %i\n", scene_no);

  if (scene_no < 0 ||
      scene_no >= g_list_length (scenes))
    {
      g_print ("accessed scene out of range, setting scene=0\n");
      scene_no = 0;
    }
  g_print ("\t%s\n", get_scene_no (scene_no)->title);

  if (current_scene >= 0)
    {
      if (get_scene_no (current_scene)->destroy)
        get_scene_no (current_scene)->destroy (get_scene_no (current_scene));
      else
        clutter_actor_destroy (get_scene_no (current_scene)->group);
    }

  if (get_scene_no (scene_no)->create)
    get_scene_no (scene_no)->create (get_scene_no (scene_no));

  current_scene = scene_no;
}

gint
scene_get_current (void)
{
  return current_scene;
}
