#include <stdlib.h>
#include <stdarg.h>
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "blockbox.h"
#include "actor-manipulator.h"
void tidy_cursor (gint x, gint Y){};
void scene_about (Scene *scene);
void scene_bridge (Scene *scene);
void scene_pyramid (Scene *scene);
void scene_slides (Scene *scene);
void scene_control (Scene *scene);
void scene_chain (Scene *scene);
void scene_distance_joint (Scene *scene);
void scene_prismatic_joint (Scene *scene);

static void
init_scenes (void)
{
  scenes_add_scene ("about", scene_about);
  scenes_add_scene ("pyramid", scene_pyramid);
  scenes_add_scene ("slides", scene_slides);
  scenes_add_scene ("bridge", scene_bridge);
  scenes_add_scene ("chain", scene_chain);
  scenes_add_scene ("distance joint", scene_distance_joint);

  /* the following are disabled because they don't quite
   * do what they were intended to do
   */
  /* scenes_add_scene ("prismatic joint", scene_prismatic_joint);
     scenes_add_scene ("control", scene_control);*/
}


void
action_previous (ClutterActor *label,
                 gpointer      userdata)
{
  scene_activate (scene_get_current () - 1);
}

void
action_next (ClutterActor *label,
             gpointer      userdata)
{
  scene_activate (scene_get_current () + 1);
}

void
action_quit (ClutterActor *label,
             gpointer      userdata)
{
  clutter_main_quit ();
}

void
action_toggle_simulating (ClutterActor *actor,
                       gpointer      userdata)
{
  simulating = !simulating;
  clutter_label_set_text (CLUTTER_LABEL (actor), simulating ? "◼" : "▶");
  clutter_box2d_set_simulating (CLUTTER_BOX2D (get_scene_no (scene_get_current ())
                                            ->group), simulating);
}

static void
stage_key_release_cb (ClutterStage           *stage,
                      ClutterKeyEvent        *kev,
                      gpointer                user_data)
{
  switch (clutter_key_event_symbol (kev))
    {
      case CLUTTER_q:    action_quit (NULL, NULL);    break;

      case CLUTTER_Left: action_previous (NULL, NULL); break;

      case CLUTTER_Right: action_next (NULL, NULL);    break;

      default:           action_next (NULL, NULL);    break;
    }
}

static gboolean
keep_on_top (ClutterActor *group)
{
  clutter_actor_raise_top (group);
  return TRUE;
}

static void
add_controls (ClutterActor *stage)
{
  ClutterActor *controls;
  controls = clutter_group_new ();
  clutter_group_add (CLUTTER_GROUP (stage), controls);
  clutter_actor_show (controls);

  wrap_group_init (CLUTTER_GROUP (controls), 10, 0, 400, 0);
  wrap_group_add_many (CLUTTER_GROUP (controls),
                       /*label_action ("Sans 30px", "q ",      "black", action_quit
                         , NULL),*/
                       label_action ("Sans 40px", "◼", "yellow",
                                     action_toggle_simulating, NULL),
                       label_action ("Sans 40px", "←", "yellow",
                                     action_previous, NULL),
                       label_action ("Sans 40px", "→ ", "yellow", action_next,
                                     NULL),
                       NULL);
  g_timeout_add (1000, keep_on_top, controls);

  g_object_set_data (G_OBJECT (controls), "_", "foo");
}

gint
main (int   argc,
      char *argv[])
{
  ClutterActor *stage;
  ClutterColor  stage_color = { 0x00, 0x00, 0x00, 0x00 };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  /*clutter_actor_set_size (stage, 320, 480);
  clutter_actor_set_size (stage, 1024, 768);*/
  clutter_actor_set_size (stage, 720, 576);


  init_scenes ();
  add_controls (stage);
  scene_activate (0);
  clutter_actor_show (stage);

  g_signal_connect (stage,
                    "key-release-event",
                    G_CALLBACK (stage_key_release_cb),
                    NULL);

  actor_manipulator_init (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}


