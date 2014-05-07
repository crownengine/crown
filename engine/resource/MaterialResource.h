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
#include "TextureResource.h"
#include "Resource.h"
#include "Vector3.h"
#include "Color4.h"
#include "Bundle.h"
#include "Allocator.h"

namespace crown
{

struct MaterialHeader
{
	uint32_t num_texture_layers;
	uint32_t texture_layers_offset;
};

/// A material describes the visual properties of a surface.
/// It is primarly intended for rendering purposes but can
/// also be used to drive other types of systems such as sounds or physics.
struct MaterialResource
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
	static void online(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& a, void* res)
	{
		CE_ASSERT_NOT_NULL(res);
		a.deallocate(res);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	uint32_t num_texture_layers() const
	{
		return ((MaterialHeader*) this)->num_texture_layers;
	}

	//-----------------------------------------------------------------------------
	ResourceId get_texture_layer(uint32_t i) const
	{
		CE_ASSERT(i < num_texture_layers(), "Index out of bounds");
		MaterialHeader* h = (MaterialHeader*) this;
		ResourceId* begin = (ResourceId*) (((char*) this) + h->texture_layers_offset);
		return begin[i];
	}

private:

	// Disable construction
	MaterialResource();
};

} // namespace crown

