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
#include "clutter-box2d-private.h"

__ClutterBox2DContactListener::
__ClutterBox2DContactListener (ClutterBox2D *box2d)
{
  this->m_box2d = box2d; 
  this->m_box2d->priv->world->SetContactListener(this);
}
  
__ClutterBox2DContactListener::~__ClutterBox2DContactListener()
{
  this->m_box2d->priv->world->SetContactListener(NULL);
}

/**
 * PreSolve is called on each collision encountered during a Step in the Box2D
 * simulation.  This callback looks up the ClutterActors that correlate
 * to the objects within the Box2D world.  It creates a ClutterBox2DCollision
 * objects and initializes them appropriately.  It then chains those objects
 * onto the list of pending collisions (to be processed after the full set of
 * simulation Steps have finished in clutter_box2d_iterate().
 */
void
__ClutterBox2DContactListener::PreSolve(b2Contact *contact, const b2Manifold *old_manifold)
{
  ClutterBox2DCollision *collision;
  ClutterActor *actor1, *actor2;
  b2WorldManifold world_manifold;
  ClutterChildMeta *child_meta;
  ClutterBox2DPrivate *priv;
  b2Manifold *manifold;
  void *tmp;
  gint i;

  manifold = contact->GetManifold();
  if (manifold->pointCount == 0)
    return;

  tmp = g_hash_table_lookup (this->m_box2d->priv->bodies, contact->GetFixtureA()->GetBody());
  if (!tmp)
    return;
  child_meta = CLUTTER_CHILD_META (tmp);
  actor1 = child_meta->actor;
  if (!actor1)
    return;

  tmp = g_hash_table_lookup (this->m_box2d->priv->bodies, contact->GetFixtureB()->GetBody());
  if (!tmp)
    return;
  child_meta = CLUTTER_CHILD_META (tmp);
  actor2 = CLUTTER_ACTOR (child_meta->actor);
  if (!actor2)
    return;

  contact->GetWorldManifold (&world_manifold);
  priv = this->m_box2d->priv;

  for (i = 0; i < manifold->pointCount; i++)
    {
      collision = CLUTTER_BOX2D_COLLISION (g_object_new (CLUTTER_TYPE_BOX2D_COLLISION, NULL));
      collision->actor1 = actor1;
      collision->actor2 = actor2;
      collision->normal.x = world_manifold.normal.x;
      collision->normal.y = world_manifold.normal.y;
      collision->normal_force = manifold->points[i].normalImpulse;
      collision->tangent_force = manifold->points[i].tangentImpulse;
      collision->id = manifold->points[i].id.key;
      collision->position.x = world_manifold.points[i].x * priv->inv_scale_factor;
      collision->position.y = world_manifold.points[i].y * priv->inv_scale_factor;

      priv->collisions = g_list_prepend(priv->collisions, collision);
    }
}
