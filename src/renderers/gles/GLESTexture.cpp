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

#include "GLESTexture.h"
#include "Image.h"
#include "Pixel.h"
#include "Log.h"
#include "GLESUtils.h"
#include "GLESTextureManager.h"
#include "ImageLoader.h"

namespace crown
{

GLESTexture::GLESTexture() :
	mTextureObject(0)
{
	glGenTextures(1, &mTextureObject);
}

GLESTexture::~GLESTexture()
{
	glDeleteTextures(1, &mTextureObject);
}

GLenum GLESTexture::GetGLTarget() const
{
	return GL_TEXTURE_2D;
}

GLuint GLESTexture::GetGLObject() const
{
	return mTextureObject;
}

GLenum GLESTexture::GetGLTextureFormat() const
{
//	int value;
//	glGetTexLevelParameteriv(GetGLTarget(), 0, GL_TEXTURE_INTERNAL_FORMAT, &value);
//	return value;
	return 0;
}

void GLESTexture::Load(const char* relativePath)
{
	LoadFromFile(relativePath);
}

void GLESTexture::LoadFromFile(const char* relativePath)
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

void GLESTexture::LoadFromFile(const char* relativePath, Color4 colorKey)
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

void GLESTexture::LoadFromFile(const char* relativePath, const char* alphaGreyscale)
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

void GLESTexture::LoadFromImage(const Image* image)
{
	GLenum target = GetGLTarget();
	glBindTexture(target, mTextureObject);

	PixelFormat imageFormat = image->GetFormat();
	GLint textureFormat = GLES::GetPixelFormat(imageFormat);

	if (mGenerateMipMaps)
	{
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(target, 0, textureFormat, image->GetWidth(), image->GetHeight(), 0,
		textureFormat, GL_UNSIGNED_BYTE, image->GetBuffer());

	mWidth = image->GetWidth();
	mHeight = image->GetHeight();
}

Image* GLESTexture::GetImage() const
{
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//	GLenum target = GetGLTarget();
//	glBindTexture(target, mTextureObject);

//	

//	GLenum glFormat = GetGLTextureFormat();
//	PixelFormat format = GetTextureFormat();
//	int bytesPerPixel = Pixel::GetBytesPerPixel(format);

//	uchar* texData = new uchar[GetWidth() * GetHeight() * bytesPerPixel];

//	glGetTexImage(target, 0, glFormat, GL_UNSIGNED_BYTE, texData);

//	Image* image = new Image(IT_2D, format, GetWidth(), GetHeight(), texData);
//	return image;
	return 0;
}

PixelFormat GLESTexture::GetTextureFormat() const
{
	return GLES::GetPixelFormatFromGLFormat(GetGLTextureFormat());
}

} // namespace crown

