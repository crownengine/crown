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
#include "Resource.h"
#include "Bundle.h"
#include "Allocator.h"
#include "File.h"
#include "Assert.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct FontHeader
{
	ResourceId material;
	uint32_t num_glyphs;
};

//-----------------------------------------------------------------------------
struct FontGlyphData
{
	uint32_t id;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	float x_offset;
	float y_offset;
	float x_advance;
};

//-----------------------------------------------------------------------------
class FontResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		(void)resource;
		// TODO
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
		// TODO
	}

	//-----------------------------------------------------------------------------
	uint32_t num_glyphs() const
	{
		return ((FontHeader*)this)->num_glyphs;
	}

	//-----------------------------------------------------------------------------
	FontGlyphData get_glyph(uint32_t index) const
	{
		CE_ASSERT(index < num_glyphs(), "Index out of bounds");

		FontGlyphData* begin = (FontGlyphData*) (((char*) this) + sizeof(FontHeader));
		return begin[index];
	}

private:

	// Disable construction
	FontResource();
};

} // namespace crown

