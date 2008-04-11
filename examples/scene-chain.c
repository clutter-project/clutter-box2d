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
  clutter_actor_show (group);
  scene->group = group;

  add_cage (group, TRUE);

  {
    gint          i;
    gint          numlinks = 32;
    ClutterActor *prev_actor;

    box = clutter_rectangle_new ();
    clutter_actor_show (box);
    clutter_actor_set_size (box, 18, 5);
    clutter_actor_set_position (box, 0, 100);
    clutter_group_add (CLUTTER_GROUP (group), box);
    clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                    group), box, CLUTTER_BOX2D_STATIC);

    prev_actor = box;

    numlinks = clutter_actor_get_width (stage)/20;

    for (i = 0; i < numlinks; ++i)
      {
        box = clutter_rectangle_new ();
        clutter_actor_show (box);
        clutter_actor_set_size (box, 18, 5);
        clutter_actor_set_position (box, 20 + 20 * i, 100);
        clutter_group_add (CLUTTER_GROUP (group), box);
        clutter_box2d_actor_set_type (CLUTTER_BOX2D (
                                        group), box, CLUTTER_BOX2D_DYNAMIC);

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
