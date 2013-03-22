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

#include "Types.h"
#include "Data.h"
#include "TextureResource.h"
#include "Vec3.h"

namespace crown
{

class VertexBuffer;
class IndexBuffer;

enum SkyboxFace
{
	SF_NORTH	= 0,
	SF_SOUTH	= 1,
	SF_EAST		= 2,
	SF_WEST		= 3,
	SF_UP		= 4,
	SF_DOWN		= 5,
	SF_COUNT
};

class Skybox
{

public:

	//! Constructor
	Skybox(const Vec3& position, bool visible);

	//! Destructor
	~Skybox();

//	void SetFace(SkyboxFace face, Texture* texture);

	virtual void Render();

private:

//	VertexData mVertexData[24];
//	FaceData mFaceData[12];

//	VertexBuffer* mVertexBuffer;
//	IndexBuffer* mIndexBuffer[6];

//	Texture* mSkyboxFace[6];
};

} // namespace crown

