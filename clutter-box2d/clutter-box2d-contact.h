/*
 * This file implements the header for the C++ class used for the
 * Box2D contact listener callback.
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * Initial author James Ketrenos <jketreno@linux.intel.com>
 *
 */
#ifndef __clutter_box2ed_contact_h__
#define __clutter_box2ed_contact_h__

#include "Box2D.h"         /* b2ContactListener, b2ContactResult */
#include "clutter-box2d.h" /* ClutterBox2D */

class __ClutterBox2DContactListener : public b2ContactListener
{
private:
  ClutterBox2D *m_box2d;
  
public:
  __ClutterBox2DContactListener(ClutterBox2D *box2d);
  ~__ClutterBox2DContactListener();
  void PreSolve(b2Contact* contact, const b2Manifold *old_manifold);
};

#endif
