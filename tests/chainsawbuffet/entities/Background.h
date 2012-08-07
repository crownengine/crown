#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include "Crown.h"

using namespace Crown;

class Background: public SceneNode
{
public:
	Background(Scene* scene, SceneNode* parent, real x, real y);
	virtual ~Background();

	virtual void Update(real dt);
	virtual void Render();

private:
	Auto<Sprite> mBgSprite;
};

#endif //__BACKGROUND_H__