#include "Zombie.h"
#include "Pg.h"
#include "Globals.h"

Zombie::Zombie(Scene* scene, Camera* camera):
	PhysicNode(scene, camera, Vec3(400, 240, 0), Angles(0, 0, 0), true, CG_ZOMBIE),
	mSpeed((real)0.05), mDirection(1.0, 0.0, 0.0)
{
	//Load the body sprite
	mBodySprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/zombie1.bmp", 4, 1, 4, 1, 1, 46, 36, 1, 1);
	for(int i = 0; i < mBodySprite->GetFrameCount(); i++)
	{
		mBodySprite->GetFrame(i)->SetOffset(13, 16);
	}
	mBodySprite->SetInterpolation(true);
	mBodySpriteAnimator.SetSprite(mBodySprite.GetPointer());
	mBodySpriteAnimator.SetFrameInterval(30);
	mBodySpriteAnimator.RandomizeFrameStart();

	//	//Load the legs sprite
	//mLegsSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/Zombie_legs.bmp", 17, 2, 10, 0, 0, 12, 12, 0, 0);
	//for(int i = 0; i < mLegsSprite->GetFrameCount(); i++)
	//{
	//	mLegsSprite->GetFrame(i)->SetOffset(6, 7);
	//}
	//mLegsSpriteAnimator.SetSprite(mLegsSprite.GetPointer());
	//mLegsSpriteAnimator.SetFrameInterval(100);
	//mLegsSpriteAnimator.SetScaleX(2.8);
	//mLegsSpriteAnimator.SetScaleY(2.4);

	SetUseIntegerCoordinatesForRendering(true);

	mPosition.x = (real)(20 + rand() % 760);
	mPosition.y = (real)(20 + rand() % 440);
	mChangeDirectionInterval = rand() % 3000 + 1000;
	real angle = (real)(Math::PI * 2 / 1000.0 * (rand() % 1000));
	mDirection = Vec3(Math::Cos(angle), Math::Sin(angle), 0.0);

	//Crown::Rectangle* r = new Crown::Rectangle();
	//r->SetFromCenterAndDimensions(Vec2::ZERO, 30, 30);
	//SetShape(r);
	Circle* c = new Circle(Vec2::ZERO, 15);
	//SetShape(c);
}

Zombie::~Zombie()
{
	
}

void Zombie::Collision(PhysicNode* other)
{
	//if (dynamic_cast<Pg*>(other) != NULL)
	//{
	//	Vec3 deltaPos = mPosition - other->GetPosition();
	//	this->SetPosition(GetPosition() + deltaPos * 0.1);
	mIsColliding = true;
	//}
}

void Zombie::Update(real dt)
{
	mBodySpriteAnimator.SetFrameInterval(30/(dt*60));
	PhysicNode::Update(dt);
}

void Zombie::Render()
{
	//Do an update
	if (mChangeDirectionInterval == 0)
	{
		real angle = (real)(Math::PI * 2 / 1000.0 * (rand() % 1000));
		mDirection = Vec3(Math::Cos(angle), Math::Sin(angle), 0.0);
		mChangeDirectionInterval = rand() % 3000 + 1000;
	}
	mChangeDirectionInterval -= 1;

	mPosition += mDirection * mSpeed;

	real angle = mDirection.ToVec2().GetAngle();
	mBodySpriteAnimator.SetAngle(angle);
	mLegsSpriteAnimator.SetAngle(angle);

	PhysicNode::Render();
	//mLegsSpriteAnimator.Render();
	mBodySpriteAnimator.Render();

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

