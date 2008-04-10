#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_about (Scene *scene)
{
  ClutterColor  color  = { 0x77, 0x99, 0xbb, 0xff };
  ClutterColor  color2 = { 0x22, 0xbb, 0x33, 0xff };
  ClutterActor *ground;
  ClutterActor *rectangle;
  ClutterActor *title;
  ClutterActor *stage;
  ClutterActor *group;

  stage = clutter_stage_get_default ();

  group = g_object_new (CLUTTER_TYPE_BOX2D, NULL);
  clutter_actor_show (group);
  clutter_group_add (CLUTTER_GROUP (stage), group);

  title = clutter_label_new_full ("Sans 20px",
                                  "This application is a collection\n"
                                  "clutter+box2d experiments, activate\n"
                                  "the arrows to change experiment.\n"
                                  "\n"
                                  "activate playing|pause to freeze.\n"
                                  "simulation.", &color);

  clutter_actor_show (title);
  clutter_actor_set_position (title, 40, 40);
  clutter_group_add (CLUTTER_GROUP (group), title);


  ground = clutter_rectangle_new ();
  clutter_actor_show (ground);
  clutter_actor_set_size (ground, 1024, 5);
  clutter_actor_set_position (ground, -300, 350);
  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (group),
                                ground, CLUTTER_BOX2D_STATIC);

  rectangle = clutter_rectangle_new ();
  clutter_actor_show (rectangle);
  clutter_actor_set_size (rectangle, 150, 150);
  clutter_actor_set_position (rectangle, 100, 120);
  clutter_actor_set_rotation (rectangle, CLUTTER_Z_AXIS, 23, 0, 0, 0);

  clutter_group_add (CLUTTER_GROUP (group), rectangle);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (group),
                                rectangle, CLUTTER_BOX2D_DYNAMIC);

  title = clutter_label_new_full ("Sans 40px", "Clutter-Box2D", &color2);
  clutter_actor_show (title);
  clutter_actor_set_position (title, 100, 120);
  clutter_actor_set_rotation (title, CLUTTER_Z_AXIS, 23, 0, 0, 0);
  clutter_group_add (CLUTTER_GROUP (group), title);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (group),
                                title, CLUTTER_BOX2D_DYNAMIC);

  clutter_box2d_set_playing (CLUTTER_BOX2D (group), playing);

  scene->group = group;
}
