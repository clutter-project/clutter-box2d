#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

void
scene_bridge (Scene *scene)
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
    gint          numplanks = 23;
    gint          y;
    ClutterActor *prev_actor;
   
    y = clutter_actor_get_height (stage)/2; 
    numplanks = clutter_actor_get_width (stage)/20 - 2;

    box = clutter_rectangle_new ();
    clutter_actor_set_size (box, 18, 5);
    clutter_actor_set_position (box, 10, y);
    clutter_group_add (CLUTTER_GROUP (group), box);

    clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                                 "mode", CLUTTER_BOX2D_STATIC, NULL);

    prev_actor = box;

    if (1) for (i = 0; i < numplanks; ++i)
        {
          box = clutter_rectangle_new ();
          clutter_actor_set_size (box, 18, 5);
          clutter_actor_set_position (box, 20 + 20 * i, y);
          clutter_group_add (CLUTTER_GROUP (group), box);

          clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                                       "manipulatable", TRUE,
                                       "mode", CLUTTER_BOX2D_DYNAMIC, NULL);

          {
            ClutterVertex anchor = {  (20 + 20 * i),
                                      (y) };
            clutter_box2d_add_revolute_joint2 (CLUTTER_BOX2D (
                                                 group), prev_actor, box,
                                               &anchor);
          }
          prev_actor = box;
        }

    box = clutter_rectangle_new ();
    clutter_actor_set_size (box, 18, 5);
    clutter_actor_set_position (box, 20 + 20 * numplanks, y);
    clutter_group_add (CLUTTER_GROUP (group), box);

    clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                                 "mode", CLUTTER_BOX2D_STATIC, NULL);

    {
      ClutterVertex anchor = {  (20 + 20 * i),
                                (y) };
      clutter_box2d_add_revolute_joint2 (CLUTTER_BOX2D (
                                           group), prev_actor, box, &anchor);
    }
  }

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);
}
