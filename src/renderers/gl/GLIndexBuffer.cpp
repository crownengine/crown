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

#include "GLIndexBuffer.h"
#include <GL/glew.h>
#include "Types.h"

namespace Crown
{

GLIndexBuffer::GLIndexBuffer()
{
	glGenBuffers(1, &mBufferID);
}

GLIndexBuffer::~GLIndexBuffer()
{
	glDeleteBuffers(1, &mBufferID);
}

void GLIndexBuffer::SetIndexData(const ushort* indexData, uint indexCount)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(ushort), indexData, GL_DYNAMIC_DRAW);
	mCount = indexCount;
}

void GLIndexBuffer::SetIndexSubData(const ushort* indexData, uint indexOffset, uint indexCount)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset * sizeof(ushort), indexCount * sizeof(ushort), indexData);
}

void GLIndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
}

void GLIndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}

