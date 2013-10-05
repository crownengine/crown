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

namespace crown
{

// All offsets are absolute
struct PackageHeader
{
	uint32_t num_textures;
	uint32_t textures_offset;
	uint32_t num_scripts;
	uint32_t scripts_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
	uint32_t num_meshes;
	uint32_t meshes_offset;
};

class PackageResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		const size_t file_size = file->size() - 12;
		PackageResource* res = (PackageResource*) allocator.allocate(sizeof(PackageResource));
		res->m_data = (char*) allocator.allocate(file_size);
		file->read(res->m_data, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);

		allocator.deallocate(((PackageResource*)resource)->m_data);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
	}

public:

	//-----------------------------------------------------------------------------
	uint32_t num_textures() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((PackageHeader*)m_data)->num_textures;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_scripts() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((PackageHeader*)m_data)->num_scripts;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_sounds() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((PackageHeader*)m_data)->num_sounds;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_meshes() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((PackageHeader*)m_data)->num_meshes;
	}

	//-----------------------------------------------------------------------------
	ResourceId get_texture_id(uint32_t i) const
	{
		CE_ASSERT(i < num_textures(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) (m_data + ((PackageHeader*)m_data)->textures_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_script_id(uint32_t i) const
	{
		CE_ASSERT(i < num_scripts(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) (m_data + ((PackageHeader*)m_data)->scripts_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_sound_id(uint32_t i) const
	{
		CE_ASSERT(i < num_sounds(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) (m_data + ((PackageHeader*)m_data)->sounds_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_mesh_id(uint32_t i) const
	{
		CE_ASSERT(i < num_meshes(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) (m_data + ((PackageHeader*)m_data)->meshes_offset);
		return begin[i];
	}

private:

	char* m_data;
};

} // namespace crown
