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

#include "GLESTextureManager.h"
#include "GLESTexture.h"

namespace Crown
{

Texture* GLESTextureManager::Create(const char* name, bool& created)
{
	Texture* texture = static_cast<GLESTexture*>(ResourceManager::Create(name, created));

	return texture;
}

Texture* GLESTextureManager::Load(const char* name, bool generateMipMaps)
{
	bool created;
	Texture* texture = Create(name, created);

	if (texture != NULL && created)
	{
		// Populate parameters
		texture->SetGenerateMipMaps(generateMipMaps);

		texture->Load(name);
	}

	return texture;
}

Texture* GLESTextureManager::Load(const char* name, bool generateMipMaps, Color4 colorKey)
{
	bool created;
	Texture* texture = Create(name, created);

	if (texture != NULL && created)
	{
		// Populate parameters
		texture->SetGenerateMipMaps(generateMipMaps);

		texture->LoadFromFile(name, colorKey);
	}

	return texture;
}

Texture* GLESTextureManager::Load(const char* name, const Str& greyscaleAlpha, bool generateMipMaps)
{
	bool created;
	Texture* texture = Create(name, created);

	if (texture != NULL && created)
	{
		// Populate parameters
		texture->SetGenerateMipMaps(generateMipMaps);

		texture->LoadFromFile(name, greyscaleAlpha);
	}

	return texture;
}

GLESTexture* GLESTextureManager::CreateSpecific(const char* name)
{
	return new GLESTexture();
}

GLESTextureManager textureMgr;
TextureManager* GetTextureManager()
{
	return &textureMgr;
}

} // namespace Crown

