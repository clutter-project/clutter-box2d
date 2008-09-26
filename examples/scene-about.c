#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

static void
collision_cb (ClutterActor          *actor,
              ClutterBox2DCollision *collision,
              gpointer               userdata)
{
  g_print ("collision reported %p,%p intensity: %f  tangent_force: %f\n",
  collision->actor1, collision->actor2,
  collision->normal_force, collision->tangent_force);
}

void
scene_about (Scene *scene)
{
  ClutterColor  color  = { 0x77, 0x99, 0xbb, 0xff };
  ClutterColor  color2 = { 0x22, 0xbb, 0x33, 0xff };
  ClutterActor *rectangle;
  ClutterActor *title;
  ClutterActor *stage;
  ClutterActor *group;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);

  title = clutter_label_new_full ("Sans 20px",
                                  "This application is a collection\n"
                                  "clutter+box2d experiments, activate\n"
                                  "use arrows to change between tests,\n"
                                  "\n"
                                  "manipulate scene with left mouse button\n"
                                  "press stop|play to control simulation\n"
                                  "right click to get context menu", &color);

  clutter_actor_set_position (title, 40, 40);
  clutter_group_add (CLUTTER_GROUP (group), title);

  add_cage (group, TRUE);

  rectangle = clutter_rectangle_new ();
  clutter_actor_set_size (rectangle, 150, 150);
  clutter_actor_set_position (rectangle, 100, 120);
  clutter_actor_set_rotation (rectangle, CLUTTER_Z_AXIS, 23, 0, 0, 0);

  clutter_group_add (CLUTTER_GROUP (group), rectangle);

  clutter_container_child_set (CLUTTER_CONTAINER (group), rectangle,
                               "manipulatable", TRUE,
                               "mode", CLUTTER_BOX2D_DYNAMIC, NULL);

  title = clutter_label_new_full ("Sans 40px", "Clutter-Box2D", &color2);
  clutter_actor_set_position (title, 100, 120);
  clutter_actor_set_rotation (title, CLUTTER_Z_AXIS, 23, 0, 0, 0);
  clutter_group_add (CLUTTER_GROUP (group), title);

  clutter_container_child_set (CLUTTER_CONTAINER (group), title,
                               "manipulatable", TRUE,
                               "mode", CLUTTER_BOX2D_DYNAMIC, NULL);

  /* report collisions involving the title */
  {
    ClutterChildMeta *child_meta;

    child_meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (group), title);
    g_signal_connect (G_OBJECT (child_meta), "collision", G_CALLBACK (collision_cb),
                      (void*)0xf00);
  }

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  scene->group = group;
}
