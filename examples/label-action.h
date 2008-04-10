#ifndef LABEL_ACTION_H
#define LABEL_ACTION_H

#include <clutter/clutter.h>

ClutterActor *label_action (const gchar *font,
                            const gchar *label,
                            const gchar *color,
                            void       (*action) (ClutterActor *label,
                                                  gpointer      userdata),
                            gpointer     userdata
                            );

#endif
