#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_control (Scene *scene)
{
  ClutterActor     *ground;
  ClutterActor     *stage;
  ClutterActor     *group;
  gint              i;

  stage = clutter_stage_get_default ();

  group = g_object_new (CLUTTER_TYPE_BOX2D, NULL);
  clutter_group_add (CLUTTER_GROUP (stage), group);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 1024, 3);
  clutter_actor_set_position (ground, -300, 700);

  clutter_group_add (CLUTTER_GROUP (group), ground);
  /* actor needs to be added to the box2d group before
   * the type can be set.
   */
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (group),
                                ground, CLUTTER_BOX2D_STATIC);
  clutter_actor_show (ground);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 256, 3);
  clutter_actor_set_position (ground, -100, 310);
  clutter_actor_set_rotation (ground, CLUTTER_Z_AXIS, 30, 128, 16, 0);
  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), ground, CLUTTER_BOX2D_STATIC);
  clutter_actor_show (ground);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 256, 3);
  clutter_actor_set_position (ground, 200, 200);
  clutter_actor_set_rotation (ground, CLUTTER_Z_AXIS, -30, 0, 0, 0);
  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), ground, CLUTTER_BOX2D_STATIC);
  clutter_actor_show (ground);

  for (i = 0; i < 20; i++)
    {
      add_hand (group, g_random_int_range (-100, 200),
                g_random_int_range (-800, 0));
    }
  clutter_actor_show (group);

  clutter_actor_set_depth (group, -600);
  clutter_actor_set_position (group, 0, -100);


  clutter_actor_set_reactive (group, TRUE);

  clutter_box2d_set_playing (CLUTTER_BOX2D (group), playing);

  scene->group = group;
}
