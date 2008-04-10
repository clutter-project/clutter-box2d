#ifndef _WRAP_GROUP_H
#define _WRAP_GROUP_H

void * wrap_group_init   (ClutterGroup *group,
                          gint          x0,
                          gint          y0,
                          gint          width,
                          gint          padding_bottom);

void wrap_group_add      (ClutterGroup *group,
                          ClutterActor *child);

void wrap_group_add_many (ClutterGroup *group,
                          ClutterActor *first_child,
                          ...);
#endif
