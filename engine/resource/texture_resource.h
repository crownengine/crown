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

#include "types.h"
#include "resource.h"
#include "bundle.h"
#include "allocator.h"
#include "file.h"
#include "resource_manager.h"
#include "memory.h"
#include <bgfx.h>

namespace crown
{

struct TextureHeader
{
	uint32_t version;
	uint32_t size;
};

struct TextureImage
{
	const bgfx::Memory* mem; // BGFX will take care of deallocation
	bgfx::TextureHandle handle;
};

struct TextureResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();
		file->skip(sizeof(TextureHeader));
		const bgfx::Memory* mem = bgfx::alloc(file_size);
		file->read(mem->data, file_size - sizeof(TextureHeader));
		bundle.close(file);

		TextureImage* teximg = (TextureImage*) default_allocator().allocate(sizeof(TextureImage));
		teximg->mem = mem;
		teximg->handle.idx = bgfx::invalidHandle;

		return teximg;
	}

	//-----------------------------------------------------------------------------
	static void online(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = TEXTURE_TYPE;
		res_id.name = id;

		TextureImage* teximg = (TextureImage*) rm.get(res_id);
		teximg->handle = bgfx::createTexture(teximg->mem);
	}

	static void offline(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = TEXTURE_TYPE;
		res_id.name = id;

		TextureImage* teximg = (TextureImage*) rm.get(res_id);
		bgfx::destroyTexture(teximg->handle);
	}
	
	//-----------------------------------------------------------------------------
	static void unload(Allocator& /*a*/, void* /*resource*/)
	{
	}

private:

	// Disable construction
	TextureResource();
};

} // namespace crown
