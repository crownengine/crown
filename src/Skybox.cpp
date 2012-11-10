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

#include "Skybox.h"
#include "Device.h"
#include "Types.h"
#include "Mat4.h"
#include "Camera.h"
#include "Texture.h"
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Renderer.h"

namespace crown
{

Skybox::Skybox(const Vec3& position, const Angles& axis, bool visible)
{
	// Setup bounding box
//	mBoundingBox.SetMin(Vec3(0.0f, 0.0f, 0.0f));
//	mBoundingBox.SetMax(Vec3(0.0f, 0.0f, 0.0f));

	// North face
	mVertexData[0] = VertexData(Vec3(-1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[1] = VertexData(Vec3(1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[2] = VertexData(Vec3(1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[3] = VertexData(Vec3(-1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	// South face
	mVertexData[4] = VertexData(Vec3(1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[5] = VertexData(Vec3(-1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[6] = VertexData(Vec3(-1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[7] = VertexData(Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	// West face
	mVertexData[8] = VertexData(Vec3(-1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[9] = VertexData(Vec3(-1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[10] = VertexData(Vec3(-1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[11] = VertexData(Vec3(-1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	// East face
	mVertexData[12] = VertexData(Vec3(1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[13] = VertexData(Vec3(1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[14] = VertexData(Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[15] = VertexData(Vec3(1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	// Up face
	mVertexData[16] = VertexData(Vec3(-1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[17] = VertexData(Vec3(1.0f, 1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[18] = VertexData(Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[19] = VertexData(Vec3(-1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	// Down face
	mVertexData[20] = VertexData(Vec3(-1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(0.0f, 0.0f), Color4::WHITE);
	mVertexData[21] = VertexData(Vec3(1.0f, -1.0f, 1.0f), Vec3(0, 0, 0), Vec2(1.0f, 0.0f), Color4::WHITE);
	mVertexData[22] = VertexData(Vec3(1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(1.0f, 1.0f), Color4::WHITE);
	mVertexData[23] = VertexData(Vec3(-1.0f, -1.0f, -1.0f), Vec3(0, 0, 0), Vec2(0.0f, 1.0f), Color4::WHITE);

	mFaceData[0] = FaceData(0, 1, 2);
	mFaceData[1] = FaceData(0, 2, 3);

	mFaceData[2] = FaceData(4, 5, 6);
	mFaceData[3] = FaceData(4, 6, 7);

	mFaceData[4] = FaceData(8, 9, 10);
	mFaceData[5] = FaceData(8, 10, 11);

	mFaceData[6] = FaceData(12, 13, 14);
	mFaceData[7] = FaceData(12, 14, 15);

	mFaceData[8] = FaceData(16, 17, 18);
	mFaceData[9] = FaceData(16, 18, 19);

	mFaceData[10] = FaceData(20, 21, 22);
	mFaceData[11] = FaceData(20, 22, 23);

	mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();
	mIndexBuffer[0] = GetDevice()->GetRenderer()->CreateIndexBuffer();
	mIndexBuffer[1] = GetDevice()->GetRenderer()->CreateIndexBuffer();
	mIndexBuffer[2] = GetDevice()->GetRenderer()->CreateIndexBuffer();
	mIndexBuffer[3] = GetDevice()->GetRenderer()->CreateIndexBuffer();
	mIndexBuffer[4] = GetDevice()->GetRenderer()->CreateIndexBuffer();
	mIndexBuffer[5] = GetDevice()->GetRenderer()->CreateIndexBuffer();

	mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_TEXTURE_COORDS | VBM_NORMAL_COORDS | VBM_COLOR_COORDS), (float*) &mVertexData[0], 24);
	mIndexBuffer[0]->SetIndexData(&mFaceData[0].vertex[0], 6);
	mIndexBuffer[1]->SetIndexData(&mFaceData[2].vertex[0], 6);
	mIndexBuffer[2]->SetIndexData(&mFaceData[4].vertex[0], 6);
	mIndexBuffer[3]->SetIndexData(&mFaceData[6].vertex[0], 6);
	mIndexBuffer[4]->SetIndexData(&mFaceData[8].vertex[0], 6);
	mIndexBuffer[5]->SetIndexData(&mFaceData[10].vertex[0], 6);
}

Skybox::~Skybox()
{
}

void Skybox::SetFace(SkyboxFace face, Texture* texture)
{
	if (face > SF_COUNT)
	{
		return;
	}

	mSkyboxFace[face] = texture;
	texture->SetWrap(TW_CLAMP);
}

void Skybox::Render()
{
	Renderer* renderer = GetDevice()->GetRenderer();

//	Camera* camera = mCreator->GetCurrentCamera();
	Mat4 mat;// = camera->GetAbsoluteTransformation();
	mat.load_identity();
	Mat4 mat2;

	mat2.load_identity();
	mat2.set_translation(mat.get_translation());

	float near;
	float far;
	float scale;

	near = 0.1f;//camera->GetNearClipDistance();
	far = 100.0f;//camera->GetFarClipDistance();
	scale = (near + far) * 0.5f;

	mat2.set_scale(Vec3(scale, scale, scale));

	renderer->_SetDepthTest(false);
	renderer->_SetDepthWrite(false);

	renderer->_SetTexturing(0, true);

	renderer->SetMatrix(MT_MODEL, mat2);

	renderer->SetTexture(0, mSkyboxFace[SF_NORTH]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[0]);

	renderer->SetTexture(0, mSkyboxFace[SF_SOUTH]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[1]);

	renderer->SetTexture(0, mSkyboxFace[SF_WEST]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[2]);

	renderer->SetTexture(0, mSkyboxFace[SF_EAST]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[3]);

	renderer->SetTexture(0, mSkyboxFace[SF_UP]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[4]);

	renderer->SetTexture(0, mSkyboxFace[SF_DOWN]);
	renderer->RenderVertexIndexBuffer(mVertexBuffer, mIndexBuffer[5]);

	renderer->_SetDepthTest(true);
	renderer->_SetDepthWrite(true);

	renderer->_SetTexturing(0, false);
}

} // namespace crown

