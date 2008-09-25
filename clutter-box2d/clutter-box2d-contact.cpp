/**
 * This file implements the header for the C++ class used for the
 * Box2D contact listener callback.
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * Initial author James Ketrenos <jketreno@linux.intel.com>
 *
 */
#include "Box2D.h"         /* b2ContactListener, b2ContactResult */
#include "clutter-box2d.h" /* ClutterBox2D */
#include "clutter-box2d-contact.h"

#define SCALE_FACTOR        0.05
#define INV_SCALE_FACTOR    (1.0/SCALE_FACTOR)

__ClutterBox2DContactListener::
__ClutterBox2DContactListener (ClutterBox2D *box2d)
{
  this->m_box2d = box2d; 
  ((b2World*)this->m_box2d->world)->SetContactListener(this);
}
  
__ClutterBox2DContactListener::~__ClutterBox2DContactListener()
{
  ((b2World*)this->m_box2d->world)->SetContactListener(NULL);
}

/**
 * Result is called on each collision encountered during a Step in the Box2D
 * simulation.  This callback looks up the ClutterActors that correlate
 * to the objects within the Box2D world.  It creates a ClutterBox2DCollision
 * object and initializes it appropriately.  It then chains that object
 * onto the list of pending collisions (to be processed after the full set of 
 * simulation Steps have finished in clutter_box2d_iterate().
 */
void
__ClutterBox2DContactListener::Result(const b2ContactResult* point)
{
  ClutterBox2DCollision *collision;
  ClutterActor *actor1, *actor2;
  void *tmp;
  ClutterChildMeta  *child_meta;
    
  tmp = g_hash_table_lookup (this->m_box2d->bodies, point->shape1->GetBody());
  if (!tmp)
    return;
  child_meta = CLUTTER_CHILD_META (tmp);
  actor1 = child_meta->actor;
  if (!actor1)
    return;
    
  tmp = g_hash_table_lookup (this->m_box2d->bodies, point->shape2->GetBody());
  if (!tmp)
    return;
  child_meta = CLUTTER_CHILD_META (tmp);
  actor2 = CLUTTER_ACTOR (child_meta->actor);
  if (!actor2)
    return;
    
  collision = CLUTTER_BOX2D_COLLISION (g_object_new (CLUTTER_TYPE_BOX2D_COLLISION, NULL));
  collision->actor1 = actor1;
  collision->actor2 = actor2;
  collision->normal.x = point->normal.x;
  collision->normal.y = point->normal.y;
  collision->normal_force = point->normalImpulse;
  collision->tangent_force = point->tangentImpulse;
  collision->id = point->id.key;
  collision->position.x = point->position.x * INV_SCALE_FACTOR;
  collision->position.y = point->position.y * INV_SCALE_FACTOR;
    
  this->m_box2d->collisions = g_list_prepend(this->m_box2d->collisions, 
					     collision);
}
