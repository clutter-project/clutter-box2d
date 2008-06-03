#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_prismatic_joint (Scene *scene)
{
  ClutterActor     *ground;
  ClutterActor     *group;
  ClutterActor     *prev_hand  = NULL;
  ClutterActor     *first_hand = NULL;
  ClutterActor     *stage;

  stage = clutter_stage_get_default ();

  first_hand = ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 500, 120);



  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);

  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_actor_set_position (ground, clutter_actor_get_width (
                                ground) * -0.3, 568);                             /*
                                                                                    this
                                                                                    is
                                                                                    wrong
                                                                                    */

  add_cage (group, TRUE);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 256, 3);
  clutter_actor_set_position (ground, -100, 310);
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

  /*add_hand (group, 100, 100);*/
  prev_hand = add_hand (group, 200, 100);

  if(0){
    ClutterVertex anchor1 = { CLUTTER_UNITS_FROM_FLOAT (0),
                              CLUTTER_UNITS_FROM_FLOAT (0) };
    ClutterVertex anchor2 = { CLUTTER_UNITS_FROM_FLOAT (0),
                              CLUTTER_UNITS_FROM_FLOAT (0) };
    ClutterVertex axis = { CLUTTER_UNITS_FROM_FLOAT (100.0),
                           CLUTTER_UNITS_FROM_FLOAT (20.0) };
    clutter_box2d_add_prismatic_joint (CLUTTER_BOX2D (group),
                                       first_hand, prev_hand,
                                       &anchor1, &anchor2,
                                       200.0, 220.0, &axis);
  }

  clutter_actor_set_depth (group, -600);
  clutter_actor_set_position (group, 0, -100);

  clutter_actor_set_reactive (group, TRUE);

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  scene->group = group;
}

