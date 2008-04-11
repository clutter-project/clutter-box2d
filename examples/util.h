/* stuff - a content retrieval and management experiment.
 *
 * Copyright Øyvind Kolås <pippin@gimp.org> 2007-2008
 * Licensed under the GPL v3 or greater.
 */

#ifndef GR_UTIL_H
#define GR_UTIL_H

#include <clutter/clutter.h>

void stuff_deferred_call          (void     (*func)(void*) ,
                                   gpointer    data);

void stuff_actor_fade_out         (ClutterActor *actor);

void stuff_actor_fade_out_destroy (ClutterActor *actor);

void stuff_actor_fade_in          (ClutterActor *actor,
                                   gint          opacity);
  
ClutterActor * add_hand (ClutterActor *group,
                         gint          x,
                         gint          y);


void add_cage (ClutterActor *group,
               gboolean      roof);
#endif
