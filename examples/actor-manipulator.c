#include <stdlib.h>
#include <stdarg.h>
#include <clutter/clutter.h>
#include <clutter-box2d/clutter-box2d.h>
#include "blockbox.h"

#define BOX2D_MANIPULATION

typedef enum
{
  NONE,
  DIRECT
#ifdef BOX2D_MANIPULATION
  , BOX2D
#endif
} ManipulationMode;

static ManipulationMode  mode              = NONE;
static ClutterActor     *manipulated_actor = NULL;
static gfloat       orig_x, orig_y;
static gfloat       start_x, start_y;
static gfloat       orig_rotation;

#ifdef BOX2D_MANIPULATION
ClutterBox2DJoint *mouse_joint = NULL;
#endif

ClutterActor *
actor_manipulator_get_victim (void)
{
  return manipulated_actor;
}

static gboolean
should_be_manipulated (ClutterActor *actor)
{
  ClutterActor *ancestor;

  if (!actor ||
      !clutter_actor_get_parent (actor))
    return FALSE;

  for (ancestor = actor; ancestor;
       ancestor = clutter_actor_get_parent (ancestor))
    {
      if (g_object_get_data (G_OBJECT (ancestor), "_") != NULL)
        return FALSE;
    }

  return TRUE;
}


static void
set_opacity (ClutterActor *actor,
             gdouble       value)
{
  clutter_actor_set_opacity (actor, value);
}

static void
set_rotation (ClutterActor *actor,
              gdouble       value)
{
  clutter_actor_set_rotation (actor, CLUTTER_Z_AXIS, value, 0, 0, 0);
}

static
gboolean
action_remove (ClutterActor *action,
               ClutterEvent *event,
               gpointer      userdata)
{
  clutter_actor_destroy (userdata);
  return FALSE;
}

static
gboolean
action_set_linear_velocity (ClutterActor *action,
                            ClutterEvent *event,
                            gpointer      userdata)
{
  ClutterActor *actor;
  ClutterBox2D *box2d;
  ClutterVertex velocity = {  (150.0f),
                              (-150.0f) };
  actor = CLUTTER_ACTOR (userdata);
  box2d = CLUTTER_BOX2D (clutter_actor_get_parent (actor));

  clutter_container_child_set (CLUTTER_CONTAINER (box2d),
                               actor, "linear-velocity", &velocity, NULL);

  return TRUE;
}


static
gboolean
action_set_dynamic (ClutterActor *action,
                    ClutterEvent *event,
                    gpointer      userdata)
{
  ClutterActor *actor;
  ClutterBox2D *box2d;

  actor = CLUTTER_ACTOR (userdata);
  box2d = CLUTTER_BOX2D (clutter_actor_get_parent (actor));

  clutter_container_child_set (CLUTTER_CONTAINER (box2d),
                               actor, "mode", CLUTTER_BOX2D_DYNAMIC, NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (box2d),
                               actor, "manipulatable", TRUE, NULL);

  return FALSE;
}

static
gboolean
action_set_static (ClutterActor *action,
                   ClutterEvent *event,
                   gpointer      userdata)
{
  ClutterActor *actor;
  ClutterBox2D *box2d;

  actor = CLUTTER_ACTOR (userdata);
  box2d = CLUTTER_BOX2D (clutter_actor_get_parent (actor));

  clutter_container_child_set (CLUTTER_CONTAINER (box2d), actor,
                               "mode", CLUTTER_BOX2D_STATIC, NULL);

  return FALSE;
}


static
gboolean
action_add_rectangle (ClutterActor *action,
                      ClutterEvent *event,
                      gpointer      userdata)
{
  ClutterActor *group = CLUTTER_ACTOR (userdata);
  ClutterActor *box;

  box = clutter_rectangle_new ();
  clutter_actor_set_size (box, 100, 100);
  clutter_actor_set_position (box, event->button.x, event->button.y);
  clutter_group_add (CLUTTER_GROUP (group), box);

  return FALSE;
}

static void
paint_circle (ClutterActor *actor)
{
  gfloat radius = MIN (clutter_actor_get_width (actor),
                       clutter_actor_get_height (actor)) / 2.f;
  cogl_set_source_color4ub (0xff, 0xff, 0xff, 0xff);
  cogl_path_arc (radius, radius, radius, radius, 0, 360);
  cogl_path_fill ();
}

static gboolean
action_add_circle (ClutterActor *action,
                   ClutterEvent *event,
                   gpointer      userdata)
{
  const ClutterColor transparent = { 0x00, 0x00, 0x00, 0x01 };
  ClutterActor *group = CLUTTER_ACTOR (userdata);
  ClutterActor *box;

  box = clutter_rectangle_new_with_color (&transparent);
  clutter_actor_set_size (box, 50, 50);
  clutter_actor_set_position (box, event->button.x, event->button.y);
  clutter_group_add (CLUTTER_GROUP (group), box);

  clutter_container_child_set (CLUTTER_CONTAINER (group),
                               box,
                               "is-circle", TRUE,
                               NULL);
  g_signal_connect (box, "paint",
                    G_CALLBACK (paint_circle), NULL);

  return FALSE;
}

static void
paint_triangle (ClutterActor *actor)
{
  gfloat width, height;

  clutter_actor_get_size (actor, &width, &height);
  cogl_set_source_color4ub (0xff, 0xff, 0xff, 0xff);

  cogl_path_move_to (width/2, 0);
  cogl_path_line_to (width, height);
  cogl_path_line_to (0, height);
  cogl_path_close ();
  cogl_path_fill ();
}

static gboolean
action_add_triangle (ClutterActor *action,
                     ClutterEvent *event,
                     gpointer      userdata)
{
  const ClutterColor transparent = { 0x00, 0x00, 0x00, 0x01 };
  ClutterActor *group = CLUTTER_ACTOR (userdata);
  ClutterVertex vertices[] =
    { { 0.5, 0.0, 0.0 },
      { 1.0, 1.0, 0.0 },
      { 0.0, 1.0, 0.0 } };
  ClutterActor *box;

  box = clutter_rectangle_new_with_color (&transparent);
  clutter_actor_set_size (box, 50, 50);
  clutter_actor_set_position (box, event->button.x, event->button.y);
  clutter_group_add (CLUTTER_GROUP (group), box);

  /* Create a triangle vertex array */
  clutter_box2d_child_set_outline (CLUTTER_BOX2D (group), box, vertices, 3);
  g_signal_connect (box, "paint",
                    G_CALLBACK (paint_triangle), NULL);

  return FALSE;
}

static
gboolean
action_add_text (ClutterActor *action,
                 ClutterEvent *event,
                 gpointer      userdata)
{
  ClutterActor *group = CLUTTER_ACTOR (userdata);
  ClutterActor *title;
  ClutterColor  color;

  clutter_color_from_string (&color, "#888");

  title = clutter_text_new_full ("Sans 30px", "fnord", &color);

  clutter_actor_set_position (title, event->button.x, event->button.y);
  clutter_group_add (CLUTTER_GROUP (group), title);
  return FALSE;
}

static
gboolean
action_add_image (ClutterActor *action,
                  ClutterEvent *event,
                  gpointer      userdata)
{
  ClutterActor *group = CLUTTER_ACTOR (userdata);
  ClutterActor *actor;

  actor = clutter_texture_new_from_file (ASSETS_DIR "redhand.png", NULL);
  clutter_actor_set_position (actor, event->button.x, event->button.y);
  clutter_group_add (CLUTTER_GROUP (group), actor);
  return FALSE;
}


#if 0
static
gboolean
action_add_block_tree (ClutterActor *action,
                       ClutterEvent *event,
                       gpointer      userdata)
{
  ClutterActor *actor;

  actor = block_tree_new ("foo");
  clutter_group_add (CLUTTER_GROUP (clutter_stage_get_default ()), actor);
  clutter_actor_set_position (actor, 20, 40);
  return FALSE;
}
#endif


static
gboolean
action_zero_gravity (ClutterActor *action,
                     ClutterEvent *event,
                     gpointer      userdata)
{
  ClutterBox2D *box2d  = CLUTTER_BOX2D (scene_get_group ());
  ClutterVertex vertex = { 0, 0, 0 };

  g_object_set (G_OBJECT (box2d), "gravity", &vertex, NULL);

  return FALSE;
}

static gboolean
actor_manipulator_press (ClutterActor *stage,
                         ClutterEvent *event,
                         gpointer      data)
{
  ClutterActor *actor;

  actor = clutter_stage_get_actor_at_pos (CLUTTER_STAGE (stage),
                                          CLUTTER_PICK_ALL,
                                          event->button.x,
                                          event->button.y);


  if (actor == stage ||
      CLUTTER_IS_GROUP (actor))
    {
      if (event->button.button == 3)
        {
          popup_nuke (stage, event->button.x, event->button.y);
          popup_add ("+rectangle", "bar", G_CALLBACK (
                       action_add_rectangle), scene_get_group ());
          popup_add ("+circle", "bar", G_CALLBACK (
                       action_add_circle), scene_get_group ());
          popup_add ("+triangle", "bar", G_CALLBACK (
                       action_add_triangle), scene_get_group ());
          popup_add ("+text", "bar", G_CALLBACK (
                       action_add_text), scene_get_group ());
          popup_add ("+image", "bar", G_CALLBACK (
                       action_add_image), scene_get_group ());
#if 0
          popup_add ("+block-tree", "bar", G_CALLBACK (
                       action_add_block_tree), scene_get_group ());
#endif
          popup_add ("zero gravity", "bar", G_CALLBACK (
                       action_zero_gravity), scene_get_group ());
        }
      return TRUE;
    }

  if (actor == NULL)
    {
      return FALSE;
    }

  if (event->button.button == 3)
    {
      popup_nuke (stage, event->button.x, event->button.y);
      popup_add ("remove", "bar", G_CALLBACK (action_remove), actor);
      popup_add ("set linear velocity", "bar", G_CALLBACK (action_set_linear_velocity), actor);
      popup_add ("set dynamic", "bar", G_CALLBACK (action_set_dynamic), actor);
      popup_add ("set static", "bar", G_CALLBACK (action_set_static), actor);
      popup_add_slider ("opacity", "hm", 0.0, 255.0,
                        clutter_actor_get_opacity (actor) * 1.0,
                        G_CALLBACK (set_opacity), actor);

      popup_add_slider ("rotation", "hm", 0.0, 360.0,
                        clutter_actor_get_rotation (actor, CLUTTER_Z_AXIS, NULL,
                                                    NULL, NULL),
                        G_CALLBACK (set_rotation), actor);

      popup_add ("ok", "bar", NULL, NULL);
      return TRUE;
    }

  if (!should_be_manipulated (actor))
    return FALSE;

  manipulated_actor = actor;

  clutter_actor_get_position (actor, &orig_x, &orig_y);
  orig_rotation = clutter_actor_get_rotation (actor, CLUTTER_Z_AXIS, NULL,
                                               NULL,
                                               NULL);

  start_x =  (event->button.x);
  start_y =  (event->button.y);

  clutter_actor_transform_stage_point (
    clutter_actor_get_parent (manipulated_actor),
    start_x, start_y,
    &start_x, &start_y);


  mode = DIRECT;


#ifdef BOX2D_MANIPULATION
  /* Use Box2D manipulation if the actor is dynamic, and the physics
   * engine is running
   */
  if (CLUTTER_IS_BOX2D (scene_get_group ()) &&
      clutter_box2d_get_simulating (CLUTTER_BOX2D (scene_get_group ())))
    {
      ClutterBox2D *box2d  = CLUTTER_BOX2D (scene_get_group ());
      /*ClutterVertex target = { start_x, start_y };*/
      gint type;
      
      clutter_container_child_get (CLUTTER_CONTAINER (box2d),
                                   manipulated_actor, "mode", &type, NULL);
      	  
      if (type == CLUTTER_BOX2D_DYNAMIC)
        {
#if 0
            mouse_joint = clutter_box2d_add_mouse_joint (CLUTTER_BOX2D (
                                                           scene_get_group ()),
                                                         manipulated_actor,
                                                         &target);
#endif
            mode = NONE; /*Box2D;*/
            manipulated_actor = NULL;
            return FALSE;
        }
    }
#endif
  clutter_set_motion_events_enabled (FALSE);

  return TRUE;
}

static gboolean
actor_manipulator_motion (ClutterActor *stage,
                          ClutterEvent *event,
                          gpointer      data)
{
  if (manipulated_actor)
    {
      gfloat x;
      gfloat y;
      gfloat dx;
      gfloat dy;

      x =  (event->button.x);
      y =  (event->button.y);

      clutter_actor_transform_stage_point (
        clutter_actor_get_parent (manipulated_actor),
        x, y,
        &x, &y);

      dx = x - start_x;
      dy = y - start_y;

      switch (mode)
        {
#ifdef BOX2D_MANIPULATION
          case BOX2D:
          {
            ClutterVertex target = { x, y };
            clutter_box2d_mouse_joint_update_target (mouse_joint, &target);
            break;
          }
#endif

          case DIRECT:
            if (clutter_event_get_state (event) & CLUTTER_BUTTON1_MASK)
              {
                x = orig_x + dx;
                y = orig_y + dy;

                clutter_actor_set_position (manipulated_actor, x, y);
              }
            else if (clutter_event_get_state (event) & CLUTTER_BUTTON2_MASK)
              {
                clutter_actor_set_rotation (manipulated_actor, CLUTTER_Z_AXIS,
                                             orig_rotation + dx, 0, 0, 0);
              }
            break;

          case NONE:
            g_print ("we shouldn't be doing %s in None mode\n", G_STRLOC);
            return FALSE;
        }
    }
  return FALSE;
}

static gboolean
actor_manipulator_release (ClutterActor *stage,
                           ClutterEvent *event,
                           gpointer      data)
{
  if (manipulated_actor)
    {
      clutter_set_motion_events_enabled (TRUE);
      manipulated_actor = NULL;

      switch (mode)
        {
#ifdef BOX2D_MANIPULATION
          case BOX2D:
          {
            if (mouse_joint)
              clutter_box2d_joint_destroy (mouse_joint);
            mouse_joint = NULL;
            break;
          }
#endif

          case DIRECT:
          case NONE:
            mode  = NONE;
            return FALSE;
            break;
        }
      mode              = NONE;
    }
  return TRUE;
}

void
actor_manipulator_init (ClutterActor *stage)
{
  g_signal_connect (stage, "button-press-event",
                    G_CALLBACK (actor_manipulator_press), NULL);
  g_signal_connect (stage, "motion-event",
                    G_CALLBACK (actor_manipulator_motion), NULL);
  g_signal_connect (stage, "button-release-event",
                    G_CALLBACK (actor_manipulator_release), NULL);
}
