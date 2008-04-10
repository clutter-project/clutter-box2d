#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

#define PYRAMID_ROWS    11

void
scene_pyramid (Scene *scene)
{
  ClutterActor *stage;
  ClutterActor *group;
  ClutterActor *box;

  stage = clutter_stage_get_default ();

  group = g_object_new (CLUTTER_TYPE_BOX2D, NULL);
  clutter_group_add (CLUTTER_GROUP (stage), group);
  clutter_actor_show (group);
  scene->group = group;

  box = clutter_rectangle_new ();
  clutter_actor_set_size (box, 1424, 4);
  clutter_actor_set_position (box, -400, 600);
  clutter_group_add (CLUTTER_GROUP (group), box);
  clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                  group), box, CLUTTER_BOX2D_STATIC);
  clutter_actor_show (box);

  {
    gint row;
    for (row = 0; row <= PYRAMID_ROWS; row++)
      {
        gint count = PYRAMID_ROWS - row + 1;
        gint x, y;
        gint i;
        y = 420 - (row + 1) * 120;

        for (i = 0; i < count; i++)
          {
            x = 160 - (count * 64 / 2) + i * 64;
            add_hand (group, x, y);
          }
      }
  }
  clutter_box2d_set_playing (CLUTTER_BOX2D (group), playing);
  clutter_actor_set_depth (group, -600);
}
