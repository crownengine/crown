#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "Crown.h"

using namespace Crown;

class Zombie: public PhysicNode
{
public:
	Zombie(Scene* scene, Camera* camera);
	virtual ~Zombie();

	virtual void Update(real dt);
	virtual void Render();

	virtual void Collision(PhysicNode* other);

private:
	Auto<Sprite> mBodySprite;
	Auto<Sprite> mLegsSprite;
	SpriteAnimator mBodySpriteAnimator;
	SpriteAnimator mLegsSpriteAnimator;
	Vec3 mDirection;
	real mSpeed;
	int mChangeDirectionInterval;
	bool mIsColliding;
};

#endif //__ZOMBIE_H__