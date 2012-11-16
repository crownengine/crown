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

namespace crown
{

enum VertexBufferMode
{
	VBM_VERTEX_ONLY		= 0,
	VBM_TEXTURE_COORDS	= 1,
	VBM_NORMAL_COORDS	= 2,
	VBM_COLOR_COORDS	= 4
};

/**
	Represents a vertex buffer to stream vertexes to the GPU.
*/
class VertexBuffer
{

public:

						VertexBuffer() : mMode(VBM_VERTEX_ONLY) {}
	virtual				~VertexBuffer() { }

	inline bool			HasTextureCoords() const { return (mMode & VBM_TEXTURE_COORDS) != 0; }
	inline bool			HasNormalCoords() const { return (mMode & VBM_NORMAL_COORDS) != 0; }
	inline bool			HasColorCoords() const { return (mMode & VBM_COLOR_COORDS) != 0; }

						//! Set the vertex data, the order of coordinates is Vertex, [Normals], [Texture], [Color]
	virtual void		SetVertexData(VertexBufferMode mode, const void* vertexData, uint32_t vertexCount) = 0;
						//! Replaces a subset of the vertex data, the order of coordinates is Vertex, [Normals], [Texture], [Color]
	virtual void		SetVertexSubData(const void* vertexData, uint32_t vertexOffset, uint32_t vertexCount) = 0;

	virtual uint32_t		GetSize() const = 0;			//!< Returns the size of the buffer
	virtual uint32_t		GetVertexCount() const = 0;		//!< Returns the number of vertices in the buffer

	virtual void		Bind() const = 0;
	virtual void		Unbind() const = 0;

protected:
	
	VertexBufferMode	mMode;
};

}

