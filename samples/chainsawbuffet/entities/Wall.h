#ifndef __WALL_H__
#define __WALL_H__

#include "Crown.h"

using namespace Crown;

class Wall: public PhysicNode
{
public:
	Wall(Scene* scene, Camera* camera, real x, real y, real w, real h);
	virtual ~Wall();

	virtual void Update(real dt);
	virtual void Render();

private:
	Texture* mStoneTexture;
	Mesh* mMesh;
};

#endif //__Wall_H__
