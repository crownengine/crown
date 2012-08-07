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

#include "ResourceManager.h"
#include "Font.h"

namespace Crown
{

/**
 * Manages font loading.
 */
class FontManager : public ResourceManager
{

public:

	FontManager();

	/** 
	 * Creates a font resource with the given name.
	 * If a resource with the same name already exists, the
	 * already existent resource will be returned.
	 * @param name The name of the resource
	 * @return A pointer to the created resource
	 */
	Font* Create(const char* name, bool& created);

	/**
	 * Loads a font resource from file.
	 * @note Generally, the name of the file determines the name of the resource and vice-versa.
	 * Here is not the case, since from a single TrueType file, multiple fonts
	 * can be generated (e.g. You may want to generate various character
	 * sizes from a single .ttf).
	 * @param name Tha name of the resource
	 * @param filename The name of the file containing the resource
	 * @param ttSize The size of characters in pixels
	 * @param ttResolution The resolution of characters in DPI (Dots Per Inch)
	 * @return A pointer to the loaded resource
	 */
	Font* Load(const char* name);

	virtual Font* CreateSpecific(const char* name);

private:

	// Disable copying
	FontManager(const FontManager&);
	FontManager& operator=(const FontManager&);
};

FontManager* GetFontManager();

} // namespace Crown

