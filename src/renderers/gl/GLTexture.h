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

#include <GL/glew.h>
#include "Types.h"
#include "Texture.h"

namespace Crown
{

class GLTexture : public Texture
{

public:

					GLTexture();
					~GLTexture();

	GLenum			GetGLTarget() const;	//!< Returns the OpenGL's texture target
	GLuint			GetGLObject() const;	//!< Returns the OpenGL's texture object
	
	GLenum			GetGLTextureFormat() const;

	virtual void	Load(const char* name);
	virtual void	Unload(const char* name, bool reload);

	void			LoadFromFile(const char* relativePath);
	void			LoadFromFile(const char* relativePath, Color4 colorKey);
	void			LoadFromFile(const char* relativePath, const char* alphaGreyscale);

	void			LoadFromImage(const Image* image);

	Image*			GetImage() const;
	PixelFormat		GetTextureFormat() const;

private:

	GLuint			mTextureObject;
};

} // namespace Crown

