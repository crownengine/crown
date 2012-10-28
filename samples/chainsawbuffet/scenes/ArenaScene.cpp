#include "ArenaScene.h"
#include "Globals.h"

ArenaScene::ArenaScene(Crown::uint windowWidth, Crown::uint windowHeight) :
	mPg(NULL), mJoystick(NULL), mParticleManager(NULL)
{
}

ArenaScene::~ArenaScene()
{
	if (mPg)
	{
		delete mPg;
	}
	for(int i = 0; i < mZombies.GetSize(); i++)
	{
		delete mZombies[i];
	}
	for(int i = 0; i < mWalls.GetSize(); i++)
	{
		delete mWalls[i];
	}
	for(int i = 0; i < mBackgrounds.GetSize(); i++)
	{
		delete mBackgrounds[i];
	}
	if (mJoystick)
	{
		delete mJoystick;
	}
	if (mParticleManager)
	{
		delete mParticleManager;
	}
}

void ArenaScene::OnLoad()
{
	Scene::OnLoad();

	Renderer* renderer = GetDevice()->GetRenderer();
	renderer->SetClearColor(Color4(0.6f, 0.6f, 0.6f, 1.0f));
	renderer->_SetLighting(false);

	//Initialize the collision groups. The nodes will insert themselves in the correct group
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_ZOMBIE, CG_ZOMBIE);
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_ZOMBIE, CG_WALL);
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_PG, CG_ZOMBIE);
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_PG, CG_WALL);
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_BULLET, CG_ZOMBIE);
	GetPhysicsManager()->AddGroupPairCollisionCheck(CG_BULLET, CG_WALL);
	
	mMainCamera = this->AddCamera(NULL, Vec3(400.0, 240.0, 240.0), Angles::ZERO, true, 90, 800.0/480.0, true);

	mBackgrounds.Append(new Background(this, mMainCamera,   0, 0));
	mBackgrounds.Append(new Background(this, mMainCamera, 256, 0));
	mBackgrounds.Append(new Background(this, mMainCamera, 512, 0));
	mBackgrounds.Append(new Background(this, mMainCamera, 768, 0));

	mBackgrounds.Append(new Background(this, mMainCamera,   0, 256));
	mBackgrounds.Append(new Background(this, mMainCamera, 256, 256));
	mBackgrounds.Append(new Background(this, mMainCamera, 512, 256));
	mBackgrounds.Append(new Background(this, mMainCamera, 768, 256));

	mPg = new Pg(this, mMainCamera);
	for(int i = 0; i < 50; i++)
	{
		mZombies.Append(new Zombie(this, mMainCamera));
	}
	mWalls.Append(new Wall(this, mMainCamera, 300, 300, 100, 100));
	mParticleManager = new ParticleManager(this, mMainCamera);
	mJoystick = new Joystick(this, mMainCamera);
}

void ArenaScene::RenderScene()
{
	Scene::RenderScene();
}


