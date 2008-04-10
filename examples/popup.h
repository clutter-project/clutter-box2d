/* stuff - a content retrieval and management experiment.
 *
 * Copyright Øyvind Kolås <pippin@gimp.org> 2007-2008
 * Licensed under the GPL v3 or greater.
 */

#ifndef _BLOCKBOX_ACTIONS_H
#define _BLOCKBOX_ACTIONS_H

void popup_nuke      (ClutterActor *stage,
                              gint         x,
                              gint         y);

void popup_add        (const gchar *name,
                              const gchar *visual,
                              GCallback    callback,
                              gpointer     user_data);

void popup_add_slider (const gchar *name,
                              const gchar *visual,
                              gfloat       min,
                              gfloat       max,
                              gfloat       initial,
                              GCallback    callback,
                              gpointer     user_data);


#endif
