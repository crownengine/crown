#ifndef __SOLID_SCENE_NODE_H__
#define __SOLID_SCENE_NODE_H__

#include "Crown.h"

using namespace Crown;

class SolidSceneNode: public PhysicNode
{
public:
	//! Constructor
	SolidSceneNode(Scene* creator, SceneNode* parent, const Vec3& position, const Angles& axis, bool visible, int collisionGroupId);

	//! Destructor
	virtual ~SolidSceneNode();

	//! Tasks to perform when registering the node for rendering
	virtual void OnRegisterForRendering();
};

#endif // __SOLID_SCENE_NODE_H__

