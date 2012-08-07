#include "Pg.h"
#include "Globals.h"
#include "Joystick.h"
#include "Bullet.h"
#include "ArenaScene.h"

Pg::Pg(Scene* scene, Camera* camera):
	PhysicNode(scene, camera, Vec3(400, 240, 0), Angles(0, 0, 0), true, CG_PG),
	mSpeed(0.0), mDirection(1.0, 0.0, 0.0), mIsReloading(false), mBulletCounter(0), mBulletInterval(30)
{
	//Load the body sprite
	mBodySprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/pg_body.bmp", 4, 1, 4, 0, 0, 46, 36, 0, 0);
	for(int i = 0; i < mBodySprite->GetFrameCount(); i++)
	{
		mBodySprite->GetFrame(i)->SetOffset(13, 16);
	}
	mBodySprite->SetInterpolation(true);
	mBodySpriteAnimator.SetSprite(mBodySprite.GetPointer());
	mBodySpriteAnimator.SetFrameInterval(0);

	//Load the reload sprite
	mReloadSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/pg_body.bmp", 4, 1, 4, 0, 36, 46, 36, 0, 0);
	for(int i = 0; i < mReloadSprite->GetFrameCount(); i++)
	{
		mReloadSprite->GetFrame(i)->SetOffset(13, 16);
	}
	mReloadSprite->SetInterpolation(true);
	mReloadSpriteAnimator.SetSprite(mReloadSprite.GetPointer());
	mReloadSpriteAnimator.SetFrameInterval(10);

	//Load the legs sprite
	mLegsSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/pg_legs.bmp", 16, 2, 10, 0, 0, 12, 10, 0, 0);
	for(int i = 0; i < mLegsSprite->GetFrameCount(); i++)
	{
		mLegsSprite->GetFrame(i)->SetOffset(6, 5);
	}
	mLegsSpriteAnimator.SetSprite(mLegsSprite.GetPointer());
	mLegsSpriteAnimator.SetFrameInterval(100);
	mLegsSpriteAnimator.SetScaleX((real)2.8);
	mLegsSpriteAnimator.SetScaleY((real)2.4);

	SetUseIntegerCoordinatesForRendering(true);
	GetDevice()->GetInputManager()->RegisterKeyboardListener(this);

	/*Crown::Rectangle* r = new Crown::Rectangle();
	r->SetFromCenterAndDimensions(Vec2::ZERO, 30, 30);
	SetShape(r);*/
	Circle* c = new Circle(Vec2::ZERO, 15);
	//SetShape(c);
}

Pg::~Pg()
{
	//delete mBodySprite;
}

void Pg::Update(real dt)
{
	//Calculate the velocity based on joystick input before the parent's update, where it will be applied
	Vec3 delta = Globals::joystick->GetDelta();
	delta.y = -delta.y;
	mSpeed = delta.GetLength() * (real)1.9;
	if (mSpeed >= 0.001)
	{
		delta.Normalize();
		mDirection = delta;
	}
	SetVelocity(mDirection * mSpeed * 3.5);

	UpdateSpriteAnimators(dt);

	if (mBulletCounter > 0)
	{
		mBulletCounter -= 1;
	}

	ArenaScene* arena = (ArenaScene*)GetCreator();
	arena->GetMainCamera()->SetPosition(Vec3(mPosition.x, mPosition.y, arena->GetMainCamera()->GetPosition().z));

	PhysicNode::Update(dt);
}

void Pg::UpdateSpriteAnimators(real dt)
{
	if (mSpeed > 0.01)
	{
		//2.8 is the x scale of the legs sprite
		real legWalkDistancePixels = (real)(18 * 2.8);
		real secondsToWalkLegDistance = legWalkDistancePixels / mSpeed;
		real legFrameDurationSeconds = secondsToWalkLegDistance / mLegsSprite->GetFrameCount();
		real bodyFrameDurationSeconds = secondsToWalkLegDistance / mBodySprite->GetFrameCount();
		mLegsSpriteAnimator.SetFrameInterval((int)(GetDevice()->GetActualFps() * legFrameDurationSeconds));
		mBodySpriteAnimator.SetFrameInterval((int)(GetDevice()->GetActualFps() * bodyFrameDurationSeconds));
	}
	else
	{
		mLegsSpriteAnimator.SetFrameInterval(0);
		mLegsSpriteAnimator.SetFrameIndex(0);
		mBodySpriteAnimator.SetFrameInterval(0);
		mBodySpriteAnimator.SetFrameIndex(0);
	}

	real angle = mDirection.ToVec2().GetAngle();
	mBodySpriteAnimator.SetAngle(angle);
	mReloadSpriteAnimator.SetAngle(angle);
	mLegsSpriteAnimator.SetAngle(angle);
}

void Pg::Render()
{
	PhysicNode::Render();
	mLegsSpriteAnimator.Render();
	if (mIsReloading)
	{
		mReloadSpriteAnimator.Render();
		if (mReloadSpriteAnimator.GetFrameIndex() == 3)
		{
			mIsReloading = false;
			mBodySpriteAnimator.SetFrameIndex(0);
		}
	}
	else
	{
		mBodySpriteAnimator.Render();
	}

	//Renderer* r = GetDevice()->GetRenderer();
	//Crown::Rectangle* rect = static_cast<Crown::Rectangle*>(GetShape());
	//
	//Vec2 min = rect->GetVertext(0) - mPosition.ToVec2();
	//Vec2 size = rect->GetSize();
	//if (mIsColliding)
	//{
	//	r->DrawRectangle(Point2(min.x, min.y), Point2(size.x, size.y), DM_BORDER, Color4::BLUE, Color4::BLUE);
	//}
	//else
	//{
	//	r->DrawRectangle(Point2(min.x, min.y), Point2(size.x, size.y), DM_BORDER);
	//}
	//mIsColliding = false;
}

void Pg::KeyPressed(const KeyboardEvent& event)
{
	if (event.key == 'C')
	{
		mIsReloading = true;
		mReloadSpriteAnimator.SetFrameIndex(0);
	}
	if (event.key == ' ')
	{
		if (mBulletCounter == 0)
		{
			Vec3 pos = mPosition + mDirection * 30;
			pos += Vec3(-mDirection.y, mDirection.x, 0.0) * 7;
			
			Bullet* b = new Bullet(GetCreator(), GetParent(), pos.x, pos.y, mDirection, 400);
			mBulletCounter = mBulletInterval;
		}
	}
}
