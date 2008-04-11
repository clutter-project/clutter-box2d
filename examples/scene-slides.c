#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_slides (Scene *scene)
{
  ClutterActor     *ground;
  ClutterActor     *group;
  GdkPixbuf        *pixbuf;
  ClutterActor     *stage;
  gint              i;
  GError           *error;

  stage = clutter_stage_get_default ();

  error  = NULL;
  pixbuf = gdk_pixbuf_new_from_file (ASSETS_DIR "mars.png", &error);
  if (error)
    {
      g_warning ("Unable to load assets/mars.png: %s", error->message);
      ground = clutter_rectangle_new ();
      clutter_actor_set_size (ground, 500, 120);
      g_object_unref (pixbuf);
    }
  else
    {
      ground = clutter_texture_new_from_pixbuf (pixbuf);
      g_object_unref (pixbuf);
    }



  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);

  clutter_group_add (CLUTTER_GROUP (group), ground);
  clutter_actor_set_position (ground, clutter_actor_get_width (
                                ground) * -0.3, 568);                             /*
                                                                                    this
                                                                                    is
                                                                                    wrong
                                                                                    */

  clutter_actor_show (ground);

  add_cage (group, FALSE);

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

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  scene->group = group;
}

