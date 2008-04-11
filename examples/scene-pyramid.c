#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

static gint PYRAMID_ROWS=8;

void
scene_pyramid (Scene *scene)
{
  ClutterActor *stage;
  ClutterActor *group;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);
  clutter_actor_show (group);
  scene->group = group;

  add_cage (group, FALSE);

  PYRAMID_ROWS = (clutter_actor_get_width (stage)/64)-1;

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
            x = clutter_actor_get_width(stage)/2 - (count * 64 / 2) + i * 64;
            add_hand (group, x, y);
          }
      }
  }
  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);
}
