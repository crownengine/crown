#ifndef __BULLET_H__
#define __BULLET_H__

#include "Crown.h"

using namespace Crown;

class Bullet: public PhysicNode
{
public:
	Bullet(Scene* scene, SceneNode* parent, real x, real y, Vec3 direction, real speed);
	virtual ~Bullet();

	virtual void Update(real dt);
	virtual void Render();
	virtual void Collision(PhysicNode* other);

private:
	Auto<Sprite> mTrailSprite;
	SpriteAnimator mTrailSpriteAnimator;
	bool mHasCollided;
};

#endif //__BULLET_H__