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

#include "TextureManager.h"
#include "GLTexture.h"

namespace crown
{

class GLTextureManager : public TextureManager
{

public:

	GLTextureManager() {}

	/**
	 * Creates a texture resource withe the given name.
	 * If a resource with the same name already exists, the
	 * already existent resource will be returned. In order
	 * to distinguish between a newly created resource or a
	 * pointer to an existing one, you have to check
	 * at the value returned by 'created'.
	 * @param name The name of the resource
	 * @param created Returns true if newly created, false otherwise
	 * @return A pointer to the created resource
	 */
	virtual Texture* Create(const char* name, bool& created);

	/**
	 * Loads a texture resource from file.
	 * The name of the file determines the name of the resource and vice-versa.
	 * @param name Tha name of the resource
	 * @return A pointer to the loaded resource
	 */
	virtual Texture* Load(const char* name, bool generateMipMaps = true);

	/**
	 * Loads a texture resource from file using color keying.
	 * The name of the file determines the name of the resource and vice-versa.
	 * @param name The name of the resource
	 * @param colorKey The color to use as transparent color
	 * @return A pointer to the loaded resource
	 */
	virtual Texture* Load(const char* name, bool generateMipMaps, Color4 colorKey);

	virtual Texture* Load(const char* name, const char* greyscaleAlpha, bool generateMipMaps);

	virtual GLTexture* CreateSpecific(const char* name);

private:

	GLTextureManager(const GLTextureManager&);
	GLTextureManager& operator=(const GLTextureManager&);
};

} // namespace crown

