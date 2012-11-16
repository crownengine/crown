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

#include "GLVertexBuffer.h"
#include <GL/glew.h>

namespace crown
{

//!Constructor
GLVertexBuffer::GLVertexBuffer():
	mSize(0), mVertexCount(0)
{
	glGenBuffers(1, &mBufferID);
}

//!Destructor
GLVertexBuffer::~GLVertexBuffer()
{
	glDeleteBuffers(1, &mBufferID);
}

void GLVertexBuffer::SetVertexData(VertexBufferMode mode, const void* vertexData, uint32_t vertexCount)
{
	mMode = mode;

	uint32_t vertexSize = sizeof(float) * 3;
	if (HasNormalCoords())
		vertexSize += sizeof(float) * 3;
	if (HasTextureCoords())
		vertexSize += sizeof(float) * 2;
	if (HasColorCoords())
		vertexSize += sizeof(float) * 4;

	mSize = vertexCount * vertexSize;
	mVertexCount = vertexCount;

	glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
	glBufferData(GL_ARRAY_BUFFER, mSize, vertexData, GL_DYNAMIC_DRAW);
}

void GLVertexBuffer::SetVertexSubData(const void* vertexData, uint32_t vertexOffset, uint32_t vertexCount)
{
	uint32_t vertexSize = sizeof(float) * 3;
	if (HasNormalCoords())
		vertexSize += sizeof(float) * 3;
	if (HasTextureCoords())
		vertexSize += sizeof(float) * 2;
	if (HasColorCoords())
		vertexSize += sizeof(float) * 4;

	if (vertexSize * vertexCount + vertexOffset > mSize)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * vertexSize, vertexCount * vertexSize, vertexData);
}

uint32_t GLVertexBuffer::GetSize() const
{
	return mSize;
}

uint32_t GLVertexBuffer::GetVertexCount() const
{
	return mVertexCount;
}

void GLVertexBuffer::Bind() const
{
	uint32_t vertexSize = sizeof(float) * 3;
	uint32_t offset = vertexSize;
	if (HasNormalCoords())
		vertexSize += sizeof(float) * 3;
	if (HasTextureCoords())
		vertexSize += sizeof(float) * 2;
	if (HasColorCoords())
		vertexSize += sizeof(float) * 4;

	glBindBuffer(GL_ARRAY_BUFFER, mBufferID);

	glVertexPointer(3, GL_FLOAT, vertexSize, (void*) 0);

	if (HasNormalCoords())
	{
		glNormalPointer(GL_FLOAT, vertexSize, (void*) offset);
		offset += sizeof(float) * 3;
	}

	if (HasTextureCoords())
	{
		glClientActiveTexture(GL_TEXTURE0);
		glTexCoordPointer(2, GL_FLOAT, vertexSize, (void*) offset);
		offset += sizeof(float) * 2;
	}

	if (HasColorCoords())
	{
		glColorPointer(4, GL_FLOAT, vertexSize, (void*) offset);
		offset += sizeof(float) * 4;
	}
}

void GLVertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}

