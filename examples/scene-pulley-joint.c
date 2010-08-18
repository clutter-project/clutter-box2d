#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

static void
paint_circle (ClutterActor *actor)
{
  gfloat radius = MIN (clutter_actor_get_width (actor),
                       clutter_actor_get_height (actor)) / 2.f;
  cogl_set_source_color4ub (0xff, 0xff, 0xff, 0xff);
  cogl_path_arc (radius, radius, radius, radius, 0, 360);
  cogl_path_fill ();
}

static ClutterActor *
create_circle (gfloat radius)
{
  const ClutterColor transp = { 0, };
  ClutterActor *circle = clutter_rectangle_new_with_color (&transp);
  clutter_actor_set_size (circle, radius * 2, radius * 2);
  g_signal_connect (circle, "paint",
                    G_CALLBACK (paint_circle), NULL);
  return circle;
}

void
scene_pulley_joint (Scene *scene)
{
  ClutterActor *stage;
  ClutterActor *box2d;

  stage = clutter_stage_get_default ();

  box2d = clutter_box2d_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), box2d);
  scene->group = box2d;

  add_cage (box2d, FALSE);

  {

    gint          i;
    ClutterVertex v1, v2, v3, v4;
    ClutterActor *joint1, *joint2, *box1, *box2;

    /* Create platform 1 */
    joint1 = create_circle (10);
    clutter_actor_set_position (joint1, 110, 250);
    clutter_container_add_actor (CLUTTER_CONTAINER (box2d), joint1);
    clutter_box2d_child_set_is_circle (CLUTTER_BOX2D (box2d), joint1, TRUE);
    clutter_box2d_child_set_mode (CLUTTER_BOX2D (box2d), joint1,
                                  CLUTTER_BOX2D_DYNAMIC);
    clutter_box2d_child_set_manipulatable (CLUTTER_BOX2D (box2d), joint1,
                                           TRUE);

    box1 = clutter_rectangle_new ();
    clutter_actor_set_size (box1, 200, 20);
    clutter_actor_set_position (box1, 10, 350);
    clutter_container_add_actor (CLUTTER_CONTAINER (box2d), box1);
    clutter_box2d_child_set_mode (CLUTTER_BOX2D (box2d), box1,
                                  CLUTTER_BOX2D_DYNAMIC);
    clutter_box2d_child_set_manipulatable (CLUTTER_BOX2D (box2d), box1,
                                           TRUE);

    add_hand (box2d, 20, 350 - 62);
    add_hand (box2d, 210 - 58, 350 - 62);

    v1 = (ClutterVertex){10, 10, 0};
    v2 = (ClutterVertex){0, 0, 0};
    v3 = (ClutterVertex){200, 0, 0};
    clutter_box2d_add_distance_joint (CLUTTER_BOX2D (box2d),
                                      joint1, box1,
                                      &v1, &v2,
                                      150, 0, 0);
    clutter_box2d_add_distance_joint (CLUTTER_BOX2D (box2d),
                                      joint1, box1,
                                      &v1, &v3,
                                      150, 0, 0);

    /* Create platform 2 */
    joint2 = create_circle (10);
    clutter_actor_set_position (joint2, 530, 250);
    clutter_container_add_actor (CLUTTER_CONTAINER (box2d), joint2);
    clutter_box2d_child_set_is_circle (CLUTTER_BOX2D (box2d), joint2, TRUE);
    clutter_box2d_child_set_mode (CLUTTER_BOX2D (box2d), joint2,
                                  CLUTTER_BOX2D_DYNAMIC);
    clutter_box2d_child_set_manipulatable (CLUTTER_BOX2D (box2d), joint2,
                                           TRUE);

    box2 = clutter_rectangle_new ();
    clutter_actor_set_size (box2, 200, 20);
    clutter_actor_set_position (box2, 430, 350);
    clutter_container_add_actor (CLUTTER_CONTAINER (box2d), box2);
    clutter_box2d_child_set_mode (CLUTTER_BOX2D (box2d), box2,
                                  CLUTTER_BOX2D_DYNAMIC);
    clutter_box2d_child_set_manipulatable (CLUTTER_BOX2D (box2d), box2,
                                           TRUE);

    add_hand (box2d, 440, 350 - 62);
    add_hand (box2d, 640 - 58, 350 - 62);

    clutter_box2d_add_distance_joint (CLUTTER_BOX2D (box2d),
                                      joint2, box2,
                                      &v1, &v2,
                                      150, 0, 0);
    clutter_box2d_add_distance_joint (CLUTTER_BOX2D (box2d),
                                      joint2, box2,
                                      &v1, &v3,
                                      150, 0, 0);

    /* Create pulley joint */
    v1 = (ClutterVertex){ 120, 260, 0 };
    v2 = (ClutterVertex){ 540, 260, 0 };
    v3 = (ClutterVertex){ 120, 50, 0};
    v4 = (ClutterVertex){ 540, 50, 0};
    clutter_box2d_add_pulley_joint (CLUTTER_BOX2D (box2d),
                                    joint1, joint2,
                                    &v1, &v2,
                                    &v3, &v4,
                                    400, 400,
                                    1.0);
  }

  clutter_box2d_set_simulating (CLUTTER_BOX2D (box2d), simulating);
}
