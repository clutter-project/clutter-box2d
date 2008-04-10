/* stuff - a content retrieval and management experiment.
 *
 * Copyright Øyvind Kolås <pippin@gimp.org> 2007-2008
 * Licensed under the GPL v3 or greater.
 */

#define FONT_SIZE    "15px"
#define PADDING      4

#include <clutter/clutter.h>
#include "wrap-group.h"

typedef struct IncrementalInfo
{
  gint x0;
  gint y0;
  gint x;
  gint y;
  gint width;
  gint max_height;
  gint padding_bottom;
} IncrementalInfo;

void *
wrap_group_init (ClutterGroup *group,
                 gint          x0,
                 gint          y0,
                 gint          width,
                 gint          padding_bottom)
{
  IncrementalInfo *info;

  info = g_object_get_data (G_OBJECT (group), "incremental-info");
  if (!info)
    {
      info = g_malloc0 (sizeof (IncrementalInfo));
      g_object_set_data (G_OBJECT (group), "incremental-info", info);
      /* XXX: add a weak reference and free the incremental info when the
       * group is destroyed
       */
    }
  info->x0             = x0;
  info->y0             = y0;
  info->width          = width;
  info->x              = x0;
  info->y              = y0;
  info->padding_bottom = padding_bottom;
  return info;
}

void
wrap_group_add (ClutterGroup *group,
                ClutterActor *child)
{
  IncrementalInfo *info;

  info = g_object_get_data (G_OBJECT (group), "incremental-info");
  if (!info)
    {
      info = wrap_group_init (group, 0, 0, 1000, 0);
    }
  gint width, height;

  clutter_group_add (group, child);

  width  = clutter_actor_get_width (child);
  height = clutter_actor_get_height (child);

  if (info->x + width > info->width)
    {
      info->x          = info->x0;
      info->y         += info->max_height + info->padding_bottom;
      info->max_height = 0;
    }

  clutter_actor_set_position (child, info->x, info->y);

  info->x += width;

  if (height > info->max_height)
    info->max_height = height;

  clutter_actor_show (child);
}

#include <stdlib.h>
#include <stdarg.h>

void
wrap_group_add_many (ClutterGroup *group,
                     ClutterActor *first_child,
                     ...)
{
  ClutterActor *child;
  va_list       var_args;

  va_start (var_args, first_child);
  child = first_child;
  while (child)
    {
      wrap_group_add (group, child);
      child = va_arg (var_args, ClutterActor*);
    }
  va_end (var_args);
}
