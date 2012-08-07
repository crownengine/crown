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

#include "Sprite.h"
#include "Types.h"
#include "Device.h"
#include "Renderer.h"
#include "Log.h"
#include "BMPImageLoader.h"
#include "TextureManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Crown
{

Sprite::Sprite():
	mInterpolate(false)
{
	mVertices[0] = VertexData(Vec3(-1.0f, -1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertices[1] = VertexData(Vec3(+1.0f, -1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertices[2] = VertexData(Vec3(+1.0f, +1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertices[3] = VertexData(Vec3(-1.0f, +1.0f, 0.0f), Vec3(0, 0, 1.0f), Vec2(0.0f, 1.0f), Color4::WHITE);

	mFaces[0] = FaceData(0, 1, 2);
	mFaces[1] = FaceData(0, 2, 3);

	mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();
	mIndexBuffer = GetDevice()->GetRenderer()->CreateIndexBuffer();

	mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_TEXTURE_COORDS | VBM_NORMAL_COORDS | VBM_COLOR_COORDS), (float*) &mVertices[0], 4);
	mIndexBuffer->SetIndexData(&mFaces[0].vertex[0], 6);
}

Sprite::~Sprite()
{
	for (int i=0; i<mFrames.GetSize(); i++)
	{
		delete mFrames[i];
	}
}

void Sprite::Clear()
{
	while (mFrames.GetSize() > 0)
	{
		RemoveFrame(0);
	}
}

void Sprite::SetInterpolation(bool interpolate)
{
	mInterpolate = interpolate;
}

void Sprite::RemoveFrame(int frameNumber)
{
	if (frameNumber >= mFrames.GetSize())
	{
		Log::E("Sprite::RemoveFrame frameNumber out of array");
		return;
	}
	delete mFrames[frameNumber];
	mFrames.Remove(frameNumber);
}

void Sprite::draw(int frameNumber)
{
	draw(frameNumber, 0.0, 1.0, 1.0);
}

void Sprite::draw(int frameNumber, real angle, real scaleX, real scaleY)
{
	if (frameNumber >= mFrames.GetSize())
	{
		Log::E("Sprite::draw frameNumber out of array");
		return;
	}

	Renderer* renderer = GetDevice()->GetRenderer();

	Frame* frame = mFrames[frameNumber];

	renderer->_SetTexturing(0, true);
	renderer->_SetTexture(0, frame->mTexture);
	renderer->_SetTextureMode(0, frame->mTexture->GetMode(), frame->mTexture->GetBlendColor());

	if (mInterpolate)
	{
		renderer->_SetTextureFilter(0, TF_LINEAR);
	}
	else
	{
		renderer->_SetTextureFilter(0, TF_NEAREST);
	}

	renderer->_SetBlending(true);

	// Setup transformation
	Mat4 transformation;

	transformation.BuildRotationZ(angle);
	transformation.SetTranslation(Vec3(-frame->mOffset.x, -frame->mOffset.y, 0.0f));

	Mat4 s;
	s.LoadIdentity();
	s.SetScale(Vec3(scaleX, scaleY, 0.0f));

	transformation = s * transformation;

	// Update vertex data
	mVertices[0].position.x = 0.0f;
	mVertices[0].position.y = 0.0f;

	mVertices[1].position.x = frame->mSize.x;
	mVertices[1].position.y = 0.0f;
	
	mVertices[2].position.x = frame->mSize.x;
	mVertices[2].position.y = frame->mSize.y;
		
	mVertices[3].position.x = 0.0f;
	mVertices[3].position.y = frame->mSize.y;
	
	mVertices[0].uv.x = frame->mTextureRect.min.x;
	mVertices[0].uv.y = frame->mTextureRect.min.y;

	mVertices[1].uv.x = frame->mTextureRect.max.x;
	mVertices[1].uv.y = frame->mTextureRect.min.y;

	mVertices[2].uv.x = frame->mTextureRect.max.x;
	mVertices[2].uv.y = frame->mTextureRect.max.y;

	mVertices[3].uv.x = frame->mTextureRect.min.x;
	mVertices[3].uv.y = frame->mTextureRect.max.y;

	// Update vertex buffer
	mVertexBuffer->SetVertexSubData((float*) &mVertices[0], 0, 4);

	// Set transformation matrix and draw
	renderer->SetMatrix(MT_MODEL, transformation);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer);

	renderer->_SetBlending(false);
	renderer->_SetTexturing(0, false);
}

void Sprite::drawFit(int frameNumber, int x, int y, int w, int h)
{
	Renderer* renderer = GetDevice()->GetRenderer();

	Frame* frame = mFrames[frameNumber];

	renderer->_SetTexturing(0, true);
	renderer->_SetTexture(0, frame->mTexture);
	renderer->_SetTextureMode(0, frame->mTexture->GetMode(), frame->mTexture->GetBlendColor());
	renderer->_SetTextureFilter(0, frame->mTexture->GetFilter());
	renderer->_SetBlending(true);

	// Setup transformation
	Mat4 transformation;

	transformation.LoadIdentity();
	transformation.SetTranslation(Vec3(-frame->mOffset.x, -frame->mOffset.y, 0.0f));

	Mat4 transformation2;

	transformation2.LoadIdentity();
	transformation2.SetTranslation(Vec3(x, y, 0.0f));

	transformation = transformation * transformation2;

	// Update vertex data
	mVertices[0].position.x = 0.0f;
	mVertices[0].position.y = 0.0f;

	mVertices[1].position.x = (float)w;
	mVertices[1].position.y = 0.0f;
	
	mVertices[2].position.x = (float)w;
	mVertices[2].position.y = (float)h;
		
	mVertices[3].position.x = 0.0f;
	mVertices[3].position.y = (float)h;
	
	mVertices[0].uv.x = frame->mTextureRect.min.x;
	mVertices[0].uv.y = frame->mTextureRect.min.y;

	mVertices[1].uv.x = frame->mTextureRect.max.x;
	mVertices[1].uv.y = frame->mTextureRect.min.y;

	mVertices[2].uv.x = frame->mTextureRect.max.x;
	mVertices[2].uv.y = frame->mTextureRect.max.y;

	mVertices[3].uv.x = frame->mTextureRect.min.x;
	mVertices[3].uv.y = frame->mTextureRect.max.y;

	// Update vertex buffer
	mVertexBuffer->SetVertexSubData((float*) &mVertices[0], 0, 4);

	// Set transformation matrix and draw
	renderer->SetMatrix(MT_MODEL, transformation);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer);

	renderer->_SetBlending(false);
	renderer->_SetTexturing(0, false);
}

Sprite* Sprite::LoadSpriteFromImage(const char* filePath)
{
	Renderer* renderer = GetDevice()->GetRenderer();
	Frame* f;
	Sprite* sprite;

	Texture* texture = GetTextureManager()->Load(filePath, false, Color4(255, 0, 255));
	texture->SetWrap(TW_CLAMP);

	f = new Frame();
	f->Set(texture);

	sprite = new Sprite();
	sprite->AddFrame(f);
	return sprite;
}

Sprite* Sprite::LoadSpriteFromImage(const char* filePath, uint tileCount, uint tileRows, uint tileColumns, uint imgX, uint imgY, uint tileSizeX, uint tileSizeY, uint tilePadX, uint tilePadY)
{
	Renderer* renderer = GetDevice()->GetRenderer();
	Frame* f;
	Sprite* sprite;

	Texture* texture = GetTextureManager()->Load(filePath, false, Color4(255, 0, 255));
	texture->SetWrap(TW_CLAMP);

	sprite = new Sprite();

	uint texHeight = texture->GetHeight();

	for(int row = 0; row < tileRows; row++)
	{
		for(int col = 0; col < tileColumns; col++)
		{
			f = new Frame();
			uint x = imgX + col * (tileSizeX + tilePadX);
			uint y = imgY + row * (tileSizeY + tilePadY);
			f->Set(texture, x, texHeight - (y + tileSizeY), x + tileSizeX, texHeight - y);
			
			sprite->AddFrame(f);
			if (sprite->GetFrameCount() == tileCount)
			{
				return sprite;
			}
		}
	}

	return sprite;
}

Sprite* Sprite::LoadSpriteFromImage(const char* filePath, const char* alphaFilePath, uint tileCount, uint tileRows, uint tileColumns, uint imgX, uint imgY, uint tileSizeX, uint tileSizeY, uint tilePadX, uint tilePadY)
{
	Renderer* renderer = GetDevice()->GetRenderer();
	Frame* f;
	Sprite* sprite;

	Texture* texture = GetTextureManager()->Load(filePath, alphaFilePath, false);
	texture->SetWrap(TW_CLAMP);

	sprite = new Sprite();

	uint texHeight = texture->GetHeight();

	for(int row = 0; row < tileRows; row++)
	{
		for(int col = 0; col < tileColumns; col++)
		{
			f = new Frame();
			uint x = imgX + col * (tileSizeX + tilePadX);
			uint y = imgY + row * (tileSizeY + tilePadY);
			f->Set(texture, x, texHeight - (y + tileSizeY), x + tileSizeX, texHeight - y);
			
			sprite->AddFrame(f);
			if (sprite->GetFrameCount() == tileCount)
			{
				return sprite;
			}
		}
	}

	return sprite;
}

} //namespace Crown

