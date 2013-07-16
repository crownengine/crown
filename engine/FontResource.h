/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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
#include "Glyph.h"
#include "Resource.h"

namespace crown
{

class Allocator;
class Bundle;

class FontResource
{
public:

	static void*		load(Allocator& allocator, Bundle& bundle, ResourceId id);
	static void			online(void* resource);
	static void			unload(Allocator& allocator, void* resource);
	static void			offline();

public:

	inline uint32_t			mat_text_heigth() { return m_max_text_height; }
	inline uint32_t			max_character_height() { return m_max_character_height; }
	inline uint32_t			max_character_width() { return m_max_character_width; }

private:

	uint32_t				m_max_text_height;
	uint32_t				m_max_character_height;
	uint32_t				m_max_character_width;
};

} // namespace crown

