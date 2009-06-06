#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_distance_joint (Scene *scene)
{
  ClutterActor     *ground;
  ClutterActor     *group;
  ClutterActor     *prev_hand  = NULL;
  ClutterActor     *first_hand = NULL;
  ClutterActor     *stage;
  gint              i;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);

  add_cage (group, TRUE);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 256, 3);
  clutter_actor_set_position (ground, 0, 310);
  clutter_actor_set_rotation (ground, CLUTTER_Z_AXIS, 30, 128, 16, 0);
  clutter_group_add (CLUTTER_GROUP (group), ground);

  clutter_container_child_set (CLUTTER_CONTAINER (group), ground,
                               "mode", CLUTTER_BOX2D_STATIC, NULL);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 256, 3);
  clutter_actor_set_position (ground, 200, 200);
  clutter_actor_set_rotation (ground, CLUTTER_Z_AXIS, -30, 0, 0, 0);
  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_container_child_set (CLUTTER_CONTAINER (group), ground,
                               "mode", CLUTTER_BOX2D_STATIC, NULL);

  {
    for (i = 0; i < 3; i++)
      {
        ClutterActor *hand;
        hand = add_hand (group, g_random_int_range (0, 200),
                         g_random_int_range (0, 10));

        if (prev_hand)
          {
            ClutterVertex anchor1 = {  (0),
                                       (0) };
            ClutterVertex anchor2 = {  (0),
                                       (0) };
            clutter_box2d_add_distance_joint (CLUTTER_BOX2D (group),
                                              prev_hand, hand,
                                              &anchor1, &anchor2,
                                              200.0, 5, 0.0);
          }
        else
          {
            first_hand = hand;
          }
        prev_hand = hand;
      }
  }


  clutter_actor_set_reactive (group, TRUE);

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  scene->group = group;
}

