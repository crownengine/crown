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

#include "GLTexture.h"
#include "ImageLoader.h"
#include "Image.h"
#include "Pixel.h"
#include "GLTextureManager.h"
#include "Log.h"
#include "GLUtils.h"
#include <GL/glew.h>

namespace crown
{

GLTexture::GLTexture() :
	mTextureObject(0)
{
}

GLTexture::~GLTexture()
{
}

GLenum GLTexture::GetGLTarget() const
{
	return GL::GetTextureTarget(mType);
}

GLuint GLTexture::GetGLObject() const
{
	return mTextureObject;
}

GLenum GLTexture::GetGLTextureFormat() const
{
	int value;
	glGetTexLevelParameteriv(GetGLTarget(), 0, GL_TEXTURE_INTERNAL_FORMAT, &value);
	return value;
}

void GLTexture::Load(const char* name)
{
	glGenTextures(1, &mTextureObject);

	LoadFromFile(name);
}

void GLTexture::Unload(const char* name, bool reload)
{
	glDeleteTextures(1, &mTextureObject);

	if (reload)
	{
		Load(name);
	}
}

void GLTexture::LoadFromFile(const char* relativePath)
{
	Image* image;

	image = ImageLoader::Load(relativePath);
	// In case of fail, use the fallback image
	if (!image)
	{
		Log::E("%s: Loading failed, using fallback.", relativePath);
		LoadFromImage(GetTextureManager()->GetFallback());
		return;
	}

	LoadFromImage(image);
	delete image;
}

void GLTexture::LoadFromFile(const char* relativePath, Color4 colorKey)
{
	Image* image;

	image = ImageLoader::Load(relativePath);
	// In case of fail, use the fallback image
	if (!image)
	{
		Log::E("%s: Loading failed, using fallback.", relativePath);
		LoadFromImage(GetTextureManager()->GetFallback());
		return;
	}

	image->ApplyColorKeying(colorKey);
	LoadFromImage(image);
	delete image;
}

void GLTexture::LoadFromFile(const char* relativePath, const char* alphaGreyscale)
{
	Image* image;

	image = ImageLoader::Load(relativePath);
	// In case of fail, use the fallback image
	if (!image)
	{
		Log::E("%s: Loading failed, using fallback.", relativePath);
		LoadFromImage(GetTextureManager()->GetFallback());
		return;
	}

	Image* greyscaleImage;
	greyscaleImage = ImageLoader::Load(alphaGreyscale);
	if (!greyscaleImage)
	{
		Log::E("%s: Loading failed, no alpha map applied.", alphaGreyscale);
	}
	else
	{
		image->ApplyGreyscaleToAlpha(greyscaleImage);
		delete greyscaleImage;
	}

	LoadFromImage(image);
	delete image;
}

void GLTexture::LoadFromImage(const Image* image)
{
	GLenum target = GetGLTarget();
	glBindTexture(target, mTextureObject);

	PixelFormat imageFormat = image->GetFormat();
	GLint textureFormat = GL::GetPixelFormat(imageFormat); 

	if (mGenerateMipMaps)
	{
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(target, 0, GL_RGBA, image->GetWidth(), image->GetHeight(), 0,
		textureFormat, GL_UNSIGNED_BYTE, image->GetBuffer());

	mWidth = image->GetWidth();
	mHeight = image->GetHeight();
}

Image* GLTexture::GetImage() const
{
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);

	GLenum target = GetGLTarget();
	glBindTexture(target, mTextureObject);

	

	GLenum glFormat = GetGLTextureFormat();
	PixelFormat format = GetTextureFormat();
	int bytesPerPixel = Pixel::GetBytesPerPixel(format);

	uchar* texData = new uchar[GetWidth() * GetHeight() * bytesPerPixel];

	glGetTexImage(target, 0, glFormat, GL_UNSIGNED_BYTE, texData);
	
	Image* image = new Image(format, GetWidth(), GetHeight(), texData);
	return image;
}

PixelFormat GLTexture::GetTextureFormat() const
{
	return GL::GetPixelFormatFromGLFormat(GetGLTextureFormat());
}

} // namespace crown

