#include "Background.h"

Background::Background(Scene* scene, SceneNode* parent, real x, real y):
	SceneNode(scene, parent, Vec3(x, y, 0), Angles(0, 0, 0), true)
{
	mBgSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/tiles.bmp");
}

Background::~Background()
{
}

void Background::Update(real dt)
{
	SceneNode::Update(dt);
}

void Background::Render()
{
	SceneNode::Render();
	mBgSprite->draw(0, 0.0, 1.0, 1.0);
}
