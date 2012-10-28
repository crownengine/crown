#include "Bullet.h"
#include "Globals.h"
#include "Joystick.h"
#include "ArenaScene.h"

Bullet::Bullet(Scene* scene, SceneNode* parent, real x, real y, Vec3 direction, real speed):
	PhysicNode(scene, parent, Vec3(x, y, 0), Angles(0, 0, 0), true, CG_BULLET), mHasCollided(false)
{
	//Load the body sprite
	mTrailSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/bullets.bmp", "res/chainsawbuffet/bullets_alpha.bmp", 1, 1, 1, 1, 1, 10, 3, 0, 0);
	for(int i = 0; i < mTrailSprite->GetFrameCount(); i++)
	{
		mTrailSprite->GetFrame(i)->SetOffset(0, 1);
	}
	mTrailSprite->SetInterpolation(true);
	mTrailSpriteAnimator.SetSprite(mTrailSprite.GetPointer());
	mTrailSpriteAnimator.SetFrameInterval(0);
	mTrailSpriteAnimator.SetScaleX(6);

	SetUseIntegerCoordinatesForRendering(true);

	SetVelocity(direction * speed);

	Circle* c = new Circle(Vec2::ZERO, 2);
	//SetShape(c);
}

Bullet::~Bullet()
{
}

void Bullet::Collision(PhysicNode* other)
{
	if (!mHasCollided)
	{
		mHasCollided = true;
		ArenaScene* arena = (ArenaScene*)GetCreator();
		ParticleManager* manager = arena->GetParticleManager();
		manager->AddEmitter(new ParticleEmitter(mPosition, 0.01, 0.2, GetVelocity().ToVec2().GetAngle() + Math::PI, Math::PI / 6));
		SetVelocity(Vec3::ZERO);
	}
}

void Bullet::Update(real dt)
{
	mTrailSpriteAnimator.SetAngle(GetVelocity().ToVec2().GetAngle() + Math::PI);
	PhysicNode::Update(dt);
}

void Bullet::Render()
{
	PhysicNode::Render();
	Renderer* r = GetDevice()->GetRenderer();
	mTrailSpriteAnimator.Render();
	r->DrawRectangle(Point2(-1, -1), Point2(2, 2), DM_FILL, Color4::BLACK, Color4::BLACK);
	//mBodySpriteAnimator.Render();
}
