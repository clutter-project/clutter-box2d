#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_table (Scene *scene)
{
  ClutterActor *stage;
  ClutterActor *group;
  ClutterVertex gravity = {0,0};
  gint          i = 42;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  add_cage (group, TRUE);

  scene->group = group;

  clutter_group_add (CLUTTER_GROUP (stage), group);
  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  g_object_set (group, "gravity", &gravity, NULL);

  while (i--)
    {
      gint x, y;
      x = clutter_actor_get_width (clutter_stage_get_default ()) / 2;
      y = clutter_actor_get_height (clutter_stage_get_default ()) * 0.8;

      clutter_container_child_set (CLUTTER_CONTAINER (scene->group),
                                   add_hand (scene->group, x, y),
                                   "manipulatable", TRUE,
                                   "mode", CLUTTER_BOX2D_DYNAMIC,
                                   NULL);
                              
    }
}
