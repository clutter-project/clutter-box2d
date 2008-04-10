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
  GdkPixbuf        *pixbuf;
  ClutterActor     *stage;
  GError           *error;

  stage = clutter_stage_get_default ();

  error  = NULL;
  pixbuf = gdk_pixbuf_new_from_file (ASSETS_DIR "mars.png", &error);
  if (error)
    {
      g_warning ("Unable to load assets/mars.png: %s", error->message);
      first_hand = ground = clutter_rectangle_new ();
      clutter_actor_set_size (ground, 500, 120);
      g_object_unref (pixbuf);
    }
  else
    {
      ground = clutter_texture_new_from_pixbuf (pixbuf);
      g_object_unref (pixbuf);
    }



  group = g_object_new (CLUTTER_TYPE_BOX2D, NULL);
  clutter_group_add (CLUTTER_GROUP (stage), group);

  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_actor_set_position (ground, clutter_actor_get_width (
                                ground) * -0.3, 568);                             /*
                                                                                    this
                                                                                    is
                                                                                    wrong
                                                                                    */

  clutter_actor_show (ground);

  ground = clutter_rectangle_new ();
  clutter_actor_set_size (ground, 1024, 3);
  clutter_actor_set_position (ground, -300, 700);
  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), ground, CLUTTER_BOX2D_STATIC);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), first_hand, CLUTTER_BOX2D_STATIC);
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

  /*add_hand (group, 100, 100);*/
  prev_hand = add_hand (group, 200, 100);

  {
    ClutterVertex anchor1 = { CLUTTER_UNITS_FROM_FLOAT (0),
                              CLUTTER_UNITS_FROM_FLOAT (0) };
    ClutterVertex anchor2 = { CLUTTER_UNITS_FROM_FLOAT (0),
                              CLUTTER_UNITS_FROM_FLOAT (0) };
    ClutterVertex axis = { CLUTTER_UNITS_FROM_FLOAT (1.0),
                           CLUTTER_UNITS_FROM_FLOAT (0.0) };
    clutter_box2d_add_prismatic_joint (CLUTTER_BOX2D (group),
                                       first_hand, prev_hand,
                                       &anchor1, &anchor2,
                                       10.0, 200.0, &axis);
  }

  clutter_actor_show (group);

  clutter_actor_set_depth (group, -600);
  clutter_actor_set_position (group, 0, -100);

  clutter_actor_set_reactive (group, TRUE);

  clutter_box2d_set_playing (CLUTTER_BOX2D (group), playing);

  scene->group = group;
}

