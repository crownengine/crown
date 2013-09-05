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

#include "Allocator.h"
#include "File.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "PackageCompiler.h"
#include "PackageResource.h"
#include "TempAllocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
PackageCompiler::PackageCompiler()
	: m_has_texture(false), m_has_lua(false), m_textures(default_allocator()), m_scripts(default_allocator())
{
}

//-----------------------------------------------------------------------------
size_t PackageCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	File* file = fs.open(resource_path, FOM_READ);

	char file_buf[4096];
	file->read(file_buf, file->size());
	fs.close(file);

	JSONParser json(file_buf);
	JSONElement root = json.root();

	// Check for resource types
	if (root.has_key("texture"))
	{
		JSONElement texture_array = root.key("texture");
		uint32_t texture_array_size = texture_array.size();

		for (uint32_t i = 0; i < texture_array_size; i++)
		{
			TempAllocator256 alloc;
			DynamicString texture_name(alloc);
			texture_name += texture_array[i].string_value();
			texture_name += ".texture";

			if (!fs.is_file(texture_name.c_str()))
			{
				Log::e("Texture '%s' does not exist.", texture_name.c_str());
				return 0;
			}

			ResourceId id;
			id.id = hash::murmur2_64(texture_name.c_str(), string::strlen(texture_name.c_str()), 0);
			m_textures.push_back(id);
		}
	}

	// Check for scripts
	if (root.has_key("lua"))
	{
		JSONElement lua_array = root.key("lua");
		//lua_array = root.key("lua");
		uint32_t lua_array_size = lua_array.size();

		for (uint32_t i = 0; i < lua_array_size; i++)
		{
			TempAllocator256 alloc;
			DynamicString lua_name(alloc);
			lua_name += lua_array[i].string_value();
			lua_name += ".lua";

			if (!fs.is_file(lua_name.c_str()))
			{
				Log::e("Lua script '%s' does not exist.", lua_name.c_str());
				return 0;
			}

			ResourceId id;
			id.id = hash::murmur2_64(lua_name.c_str(), string::strlen(lua_name.c_str()), 0);
			m_scripts.push_back(id);
		}
	}

	return sizeof(PackageHeader) +
			m_textures.size() * sizeof(ResourceId) +
			m_scripts.size() * sizeof(ResourceId);
}

//-----------------------------------------------------------------------------
void PackageCompiler::write_impl(File* out_file)
{
	PackageHeader header;
	header.num_textures = m_textures.size();
	header.num_scripts = m_scripts.size();

	header.textures_offset = sizeof(PackageHeader);
	header.scripts_offset  = header.textures_offset + sizeof(ResourceId) * header.num_textures;

	out_file->write((char*) &header, sizeof(PackageHeader));

	if (m_textures.size() > 0)
	{
		out_file->write((char*) m_textures.begin(), sizeof(ResourceId) * header.num_textures);		
	}
	if (m_scripts.size() > 0)
	{
		out_file->write((char*) m_scripts.begin(), sizeof(ResourceId) * header.num_scripts);
	}

	// Cleanup
	m_textures.clear();
	m_scripts.clear();
}

} // namespace crown
