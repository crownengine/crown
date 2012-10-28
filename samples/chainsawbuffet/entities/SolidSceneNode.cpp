#include "SolidSceneNode.h"

using namespace Crown;

SolidSceneNode::SolidSceneNode(Scene* creator, SceneNode* parent, const Vec3& position, const Angles& axis, bool visible, int collisionGroupId) :
	PhysicNode(creator, parent, position, axis, visible, collisionGroupId)
{
	//This code should go in SceneNode? (when moving it there, check also the factory methods in Scene, such as AddCamera, AddSceneNode...)
	if (parent == NULL)
	{
		creator->GetRootSceneNode()->AddChild(this);
	}
}

SolidSceneNode::~SolidSceneNode()
{

}

void SolidSceneNode::OnRegisterForRendering()
{
	if (this->IsVisible())
	{
		if (mCreator)
		mCreator->RegisterNodeForRendering(this, RP_GUI);

		SceneNode::OnRegisterForRendering();
	}
}


