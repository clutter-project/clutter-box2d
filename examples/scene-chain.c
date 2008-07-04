#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_chain (Scene *scene)
{
  ClutterActor *stage;
  ClutterActor *group;
  ClutterActor *box;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);
  scene->group = group;

  add_cage (group, TRUE);

  {
    gint          i;
    gint          y;
    gint          numlinks = 32;
    ClutterActor *prev_actor;

    y = 50;
    box = clutter_rectangle_new ();
    clutter_actor_set_size (box, 18, 5);
    clutter_actor_set_position (box, clutter_actor_get_width(stage)/2, y);
    clutter_group_add (CLUTTER_GROUP (group), box);

    clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                                 "mode", CLUTTER_BOX2D_STATIC, NULL);

    prev_actor = box;

    numlinks = clutter_actor_get_height (stage)/20;
    if (clutter_actor_get_width (stage)/20 < numlinks)
      {
        numlinks = clutter_actor_get_width (stage)/20;
      }

    for (i = 0; i < numlinks; ++i)
      {
        box = clutter_rectangle_new ();
        clutter_actor_set_size (box, 18, 5);
        clutter_actor_set_position (box, 20 + 20 * i, y+=1);
        clutter_group_add (CLUTTER_GROUP (group), box);

        clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                                       "manipulatable", TRUE,
                                     "mode", CLUTTER_BOX2D_DYNAMIC, NULL);

        {
          ClutterVertex anchor1 = { CLUTTER_UNITS_FROM_FLOAT (18.0),
                                    CLUTTER_UNITS_FROM_FLOAT (0.0) };
          ClutterVertex anchor2 = { CLUTTER_UNITS_FROM_FLOAT (0.0),
                                    CLUTTER_UNITS_FROM_FLOAT (0.0) };
          clutter_box2d_add_revolute_joint (CLUTTER_BOX2D (group),
                                            prev_actor, box,
                                            &anchor1,
                                            &anchor2,
                                            0.0
                                            );
        }

        prev_actor = box;
      }
  }

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);
}
