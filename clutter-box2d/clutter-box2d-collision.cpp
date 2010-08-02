/**
 * This file provides the implementation for the 
 * ClutterBox2DCollision object passed to the signal handlers for the 
 * "collision" signal found on ClutterBox2DActor.
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * Original code authored by James Ketrenos <jketreno@linux.intel.com>
 *
 */
#include "Box2D.h"
#include <clutter/clutter.h>
#include "clutter-box2d.h"
#include "math.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "clutter-box2d-collision.h"

G_DEFINE_TYPE (ClutterBox2DCollision, clutter_box2d_collision, G_TYPE_OBJECT);

static void
clutter_box2d_collision_class_init (ClutterBox2DCollisionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  /* TODO: Register property handlers for the various collision 
   * members */
}

static void
clutter_box2d_collision_init (ClutterBox2DCollision *self)
{
  /* Object initialization... */
}

