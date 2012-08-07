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
#include "Frame.h"
#include "List.h"
#include "Data.h"

namespace Crown
{

class VertexBuffer;
class IndexBuffer;

class Sprite
{
public:
	Sprite();
	~Sprite();

	void draw(int frameNumber);
	void draw(int frameNumber, real angle, real scaleX, real scaleY);
	void drawFit(int frameNumber, int x, int y, int w, int h);

	inline void AddFrame(Frame* frame)
	{
		mFrames.Append(frame);
	}
	void RemoveFrame(int frameNumber);
	void Clear();
	inline Frame* GetFrame(int frameNumber)
	{
		return mFrames[frameNumber];
	}
	inline int GetFrameCount()
	 { return mFrames.GetSize(); }

	void SetInterpolation(bool interpolate);

	//! Load a sprite from image
	static Sprite* LoadSpriteFromImage(const char* filePath);

	//! Load a tiled sprite from an image containing the sprite sheet. imgX and imgY are in image coordinates (not cartesian)
	static Sprite* LoadSpriteFromImage(const char* filePath, uint tileCount, uint tileRows, uint tileColumns, uint imgX, uint imgY, uint tileSizeX, uint tileSizeY, uint tilePadX, uint tilePadY);

	//! Load a tiled sprite from an image containing the sprite sheet plus greyscale alpha.
	static Sprite* LoadSpriteFromImage(const char* filePath, const char* alphaFilePath, uint tileCount, uint tileRows, uint tileColumns, uint imgX, uint imgY, uint tileSizeX, uint tileSizeY, uint tilePadX, uint tilePadY);


private:

	List<Frame*>	mFrames;
	bool			mInterpolate;

	VertexData		mVertices[4];
	FaceData		mFaces[2];

	VertexBuffer*	mVertexBuffer;
	IndexBuffer*	mIndexBuffer;

};

} //namespace Crown
