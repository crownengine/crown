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
#include "Resource.h"
#include "PixelFormat.h"
#include "Texture.h"

namespace crown
{

class Bundle;
class Allocator;

class TextureResource
{
public:

	static void*		load(Allocator& allocator, Bundle& bundle, ResourceId id);
	static void			online(void* resource);
	static void			unload(Allocator& allocator, void* resource);
	static void			offline();

public:

	PixelFormat			format() const { return m_format; }
	uint16_t			width() const { return m_width; }
	uint16_t			height() const { return m_height; }
	const uint8_t*		data() const { return m_data; }

private:

	PixelFormat			m_format;
	uint16_t			m_width;
	uint16_t			m_height;
	uint8_t*			m_data;
};

} // namespace crown
