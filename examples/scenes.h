#ifndef _SCENES_H
#define _SCENES_H

#include <clutter/clutter.h>

typedef struct Scene
{
  gchar *title;
  void (*create)  (struct Scene *scene);
  void (*destroy) (struct Scene *scene);

  /* for use with create and destroy */
  ClutterActor *group;
} Scene;

gint          scene_get_current (void);
Scene        *get_scene_no      (gint            no);
void          scenes_add_scene  (const gchar *title,
                                 void       (*create)(Scene*scene));
ClutterActor *scene_get_group   (void);
void          scene_activate    (gint scene_no);

#endif
