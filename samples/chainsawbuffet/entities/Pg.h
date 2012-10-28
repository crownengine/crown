#ifndef __PG_H__
#define __PG_H__

#include "Crown.h"

using namespace Crown;

class Pg: public PhysicNode, public KeyboardListener
{
public:
	Pg(Scene* scene, Camera* camera);
	virtual ~Pg();

	virtual void Update(real dt);
	virtual void Render();

	virtual void KeyPressed(const KeyboardEvent& event);

private:
	Auto<Sprite> mBodySprite;
	Auto<Sprite> mReloadSprite;
	Auto<Sprite> mLegsSprite;
	SpriteAnimator mBodySpriteAnimator;
	SpriteAnimator mReloadSpriteAnimator;
	SpriteAnimator mLegsSpriteAnimator;
	Vec3 mDirection;
	real mSpeed;
	bool mIsColliding;

	bool mIsReloading;

	//bullet handling
	int mBulletCounter;
	int mBulletInterval;

	void UpdateSpriteAnimators(real dt);
};

#endif //__PG_H__