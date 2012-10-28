#ifndef __ARENA_SCENE_H__
#define __ARENA_SCENE_H__

#include "Pg.h"
#include "Zombie.h"
#include "Joystick.h"
#include "Wall.h"
#include "Background.h"

using namespace Crown;

class ArenaScene: public Scene
{
public:
	ArenaScene(Crown::uint windowWidth, Crown::uint windowHeight);
	virtual ~ArenaScene();

	virtual void OnLoad();
	
	virtual void RenderScene();

	inline Camera* GetMainCamera() const
	 { return mMainCamera; }

	inline ParticleManager* GetParticleManager() const
	 { return mParticleManager; }

private:
	Camera* mMainCamera;
	Pg* mPg;
	List<Zombie*> mZombies;
	List<Wall*> mWalls;
	List<Background*> mBackgrounds;
	Joystick* mJoystick;
	ParticleManager* mParticleManager;
}; 

#endif //__ARENA_SCENE_H__
