/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Vec3.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Sprite.h"
#include "Device.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Data.h"
#include "Random.h"
#include <ctime>

namespace crown
{

struct EntityId
{
	uint		innerId;
	uint		index;
};

struct Entity
{
				Entity() : mSprite(NULL), mLocalTransform(Mat4::IDENTITY)
				{
					static Random random(time(0));
					mVelocity = Vec3(random.GetUnitFloat(), random.GetUnitFloat(), random.GetUnitFloat());
				}

	EntityId	mId;

	Vec3		mVelocity;
	Mat4		mLocalTransform;
	Sprite*		mSprite;
};

#define MAX_ENTITIES 64 * 1024

class System
{
public:

				System();
				~System();

	EntityId	CreateEntity();
	void		DestroyEntity(EntityId id);
	Entity&		GetEntity(EntityId id);

	void		Update(uint dt);

private:

	uint		mNextId;					// Next available Id
	uint		mLastIndex;					// Count of entities
	uint		mFreelist;					// Start index of the freelist
	Entity		mEntities[MAX_ENTITIES];	// Entities

	VertexData		mVertices[4];
	FaceData		mFaces[2];

	VertexBuffer*	mVertexBuffer;
	IndexBuffer*	mIndexBuffer;
};

//-----------------------------------------------------------------------------
inline System::System() : mNextId(0), mLastIndex(0), mFreelist(MAX_ENTITIES)
{
	mVertices[0] = VertexData(Vec3(-1.0f, -1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(0.0f, 0.0f), Color4::RED);
	mVertices[1] = VertexData(Vec3(+1.0f, -1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(1.0f, 0.0f), Color4::RED);
	mVertices[2] = VertexData(Vec3(+1.0f, +1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(1.0f, 1.0f), Color4::RED);
	mVertices[3] = VertexData(Vec3(-1.0f, +1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(0.0f, 1.0f), Color4::RED);

	mFaces[0] = FaceData(0, 1, 2);
	mFaces[1] = FaceData(0, 2, 3);

	mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();
	mIndexBuffer = GetDevice()->GetRenderer()->CreateIndexBuffer();

	mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_TEXTURE_COORDS | VBM_NORMAL_COORDS | VBM_COLOR_COORDS), (float*) &mVertices[0], 4);
	mIndexBuffer->SetIndexData(&mFaces[0].vertex[0], 6);
}

//-----------------------------------------------------------------------------
inline System::~System()
{
}

//-----------------------------------------------------------------------------
inline EntityId System::CreateEntity()
{
	EntityId id;
	id.innerId = mNextId++;

	if (mFreelist == MAX_ENTITIES)
	{
		id.index = mLastIndex++;

		mEntities[id.index].mId = id;
	}
	else
	{
		id.index = mFreelist;
		mFreelist = mEntities[mFreelist].mId.index;
	}

	return id;
}

//-----------------------------------------------------------------------------
inline void System::DestroyEntity(EntityId id)
{
	mEntities[id.index].mId.innerId = MAX_ENTITIES;
	mEntities[id.index].mId.index = mFreelist;
	mFreelist = id.index;
}

//-----------------------------------------------------------------------------
inline Entity& System::GetEntity(EntityId id)
{
	return mEntities[id.index];
}

void System::Update(uint dt)
{
	for (uint i = 0; i < mLastIndex; i++)
	{
		if (mEntities[i].mId.innerId == MAX_ENTITIES)
		{
			//continue;
		}

		Renderer* renderer = GetDevice()->GetRenderer();

		mEntities[i].mLocalTransform.SetTranslation(mEntities[i].mLocalTransform.GetTranslation() + mEntities[i].mVelocity * (float)dt/1000.0f);
		renderer->SetMatrix(MT_MODEL, mEntities[i].mLocalTransform);
		renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer);
	}
}

} // namespace crown

