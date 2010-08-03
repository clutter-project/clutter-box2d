/* clutter-box2d - Clutter box2d integration
 *
 * This file implements a the ClutterBox2DActor class which tracks the
 * physics simulation state of an actor. Every actor in a ClutterBox2D
 * container has an assoicated such object for synchronizing visual/physical state.
 *
 * Copyright 2008 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the LGPL v2 or greater.
 */


#ifndef __CLUTTER_BOX2D_UTIL_H_
#define __CLUTTER_BOX2D_UTIL_H_

#include <clutter/clutter.h>

typedef enum { 
  CLUTTER_BOX2D_TRACK_POSITION = 1 << 0,
  CLUTTER_BOX2D_TRACK_ROTATION = 1 << 1,
  CLUTTER_BOX2D_TRACK_ALL      = 0xff
} ClutterBox2DTrackFlags;

void clutter_box2d_actor_track (ClutterActor           *actor,
                                ClutterActor           *other,
                                ClutterBox2DTrackFlags  flags);
                                
#endif
