#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"

static const ClutterColor transp = { 0, };
static gfloat last_height, height_add;
static gint global_offset;

static void
paint_outline (ClutterActor *actor)
{
  guint i, n_vertices;
  gfloat width, height;
  const ClutterVertex *outline;

  ClutterBox2D *box2d = CLUTTER_BOX2D (clutter_actor_get_parent (actor));

  outline = clutter_box2d_child_get_outline (box2d, actor, &n_vertices);
  clutter_actor_get_size (actor, &width, &height);

  cogl_path_move_to (outline[0].x * width, outline[0].y * height);
  for (i = 1; i < n_vertices; i++)
    cogl_path_line_to (outline[i].x * width, outline[i].y * height);
  cogl_path_close ();

  cogl_set_source_color4ub (0x00, 0xd0, 0x00, 0xff);
  cogl_path_fill ();
}

static void
paint_circle (ClutterActor *actor)
{
  gfloat radius = MIN (clutter_actor_get_width (actor),
                       clutter_actor_get_height (actor)) / 2.f;
  cogl_set_source_color4ub (0xe0, 0xe0, 0xe0, 0xff);
  cogl_path_arc (radius, radius, radius, radius, 0, 360);
  cogl_path_fill ();
}

static ClutterActor *
create_circle (gfloat radius)
{
  ClutterActor *circle = clutter_rectangle_new_with_color (&transp);
  clutter_actor_set_size (circle, radius * 2, radius * 2);
  g_signal_connect (circle, "paint",
                    G_CALLBACK (paint_circle), NULL);
  return circle;
}

static void
create_segment (ClutterBox2D *box2d, gfloat x)
{
  ClutterActor *segment;
  ClutterVertex outline[4];

  outline[0] = (ClutterVertex){ 0, last_height, 0 };
  last_height = CLAMP (last_height + height_add, 0.0, 0.5);
  outline[1] = (ClutterVertex){ 1, last_height, 0 };
  outline[2] = (ClutterVertex){ 1, 1, 0 };
  outline[3] = (ClutterVertex){ 0, 1, 0 };

  height_add = CLAMP (CLAMP (height_add + g_random_double_range (-0.02, 0.02),
                             -0.07, 0.07),
                      -last_height, 1.0 - last_height);

  segment = clutter_rectangle_new_with_color (&transp);
  g_signal_connect (segment, "paint",
                    G_CALLBACK (paint_outline), NULL);

  clutter_actor_set_position (segment, x, 300);
  clutter_actor_set_size (segment, 20, 300);
  clutter_container_add_actor (CLUTTER_CONTAINER (box2d), segment);

  clutter_container_child_set (CLUTTER_CONTAINER (box2d), segment,
                               "mode", CLUTTER_BOX2D_STATIC,
                               "friction", 0.8,
                               NULL);
  clutter_box2d_child_set_outline (box2d, segment, outline, 4);
}

static ClutterActor *
create_car (ClutterBox2D *box2d)
{
  ClutterVertex v1;
  ClutterBox2DJoint *joint;
  ClutterActor *body, *wheel1, *wheel2;

  body = clutter_rectangle_new ();
  clutter_actor_set_size (body, 120, 30);
  clutter_actor_set_position (body, 0, 220);

  wheel1 = create_circle (20);
  wheel2 = create_circle (20);

  clutter_container_add (CLUTTER_CONTAINER (box2d),
                         body, wheel1, wheel2, NULL);

  clutter_container_child_set (CLUTTER_CONTAINER (box2d), body,
                               /*"manipulatable", TRUE,*/
                               "mode", CLUTTER_BOX2D_DYNAMIC, NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (box2d), wheel1,
                               "mode", CLUTTER_BOX2D_DYNAMIC,
                               "friction", 0.8,
                               "is-circle", TRUE, NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (box2d), wheel2,
                               "mode", CLUTTER_BOX2D_DYNAMIC,
                               "friction", 0.8,
                               "is-circle", TRUE, NULL);

  clutter_actor_set_position (wheel1, 0, 230);
  clutter_actor_set_position (wheel2, 80, 230);

  v1 = (ClutterVertex){ 20, 250, 0 };
  joint = clutter_box2d_add_revolute_joint2 (box2d,
                                             body, wheel1,
                                             &v1);
  clutter_box2d_joint_set_engine (joint, TRUE, 50000, 12);

  v1 = (ClutterVertex){ 100, 250, 0 };
  joint = clutter_box2d_add_revolute_joint2 (box2d,
                                             body, wheel2,
                                             &v1);
  clutter_box2d_joint_set_engine (joint, TRUE, 50000, 12);

  return body;
}

static void
paint_cb (ClutterActor *car)
{
  cogl_translate (MIN (0, -(clutter_actor_get_x (car) - 400.f)), 0, 0);
}

static void
scroll_cb (ClutterBox2D *box2d,
           ClutterActor *car)
{
  GList *to_destroy, *c, *children;
  gint offset = clutter_actor_get_x (car) - 400.f;

  to_destroy = NULL;
  children = clutter_container_get_children (CLUTTER_CONTAINER (box2d));
  for (c = children; c; c = c->next)
    {
      ClutterActor *child = c->data;
      if (clutter_actor_get_x (child) +
          clutter_actor_get_width (child) < offset)
        to_destroy = g_list_prepend (to_destroy, child);
    }
  g_list_free (children);

  while (to_destroy)
    {
      clutter_actor_destroy (CLUTTER_ACTOR (to_destroy->data));
      to_destroy = g_list_delete_link (to_destroy, to_destroy);
    }

  while (global_offset < offset)
    {
      create_segment (box2d, 800 + global_offset);
      global_offset += 20;
    }
}

void
scene_car (Scene *scene)
{
  gint i;
  ClutterActor *stage, *group, *car;

  stage = clutter_stage_get_default ();

  group = clutter_box2d_new ();
  clutter_group_add (CLUTTER_GROUP (stage), group);
  scene->group = group;

  car = create_car (CLUTTER_BOX2D (group));

  global_offset = 0;
  last_height = height_add = 0;
  for (i = 0; i < 800; i += 20)
    create_segment (CLUTTER_BOX2D (group), i);

  clutter_box2d_set_simulating (CLUTTER_BOX2D (group), simulating);

  g_signal_connect_swapped (group, "paint",
                            G_CALLBACK (paint_cb), car);
  g_signal_connect_swapped (group, "pick",
                            G_CALLBACK (paint_cb), car);
  g_signal_connect_after (group, "paint",
                          G_CALLBACK (scroll_cb), car);
}

