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
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
PackageCompiler::PackageCompiler()
	: m_has_texture(false)
	, m_has_lua(false)
	, m_has_sound(false)
	, m_has_mesh(false)
	, m_has_sprite(false)
	, m_texture(default_allocator())
	, m_script(default_allocator())
	, m_sound(default_allocator())
	, m_mesh(default_allocator())
	, m_sprite(default_allocator())
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
			m_texture.push_back(id);
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
			m_script.push_back(id);
		}
	}

	// Check for sounds
	if (root.has_key("sound"))
	{
		JSONElement sound_array = root.key("sound");
		uint32_t sound_array_size = sound_array.size();

		for (uint32_t i = 0; i < sound_array_size; i++)
		{
			TempAllocator256 alloc;
			DynamicString sound_name(alloc);
			sound_name += sound_array[i].string_value();
			sound_name += ".sound";

			if (!fs.is_file(sound_name.c_str()))
			{
				Log::e("Sound '%s' does not exist.", sound_name.c_str());
				return 0;
			}

			ResourceId id;
			id.id = hash::murmur2_64(sound_name.c_str(), string::strlen(sound_name.c_str()), 0);
			m_sound.push_back(id);
		}
	}

	// Check for meshes
	if (root.has_key("mesh"))
	{
		JSONElement mesh_array = root.key("mesh");
		uint32_t mesh_array_size = mesh_array.size();

		for (uint32_t i = 0; i < mesh_array_size; i++)
		{
			TempAllocator256 alloc;
			DynamicString mesh_name(alloc);
			mesh_name += mesh_array[i].string_value();
			mesh_name += ".mesh";

			if (!fs.is_file(mesh_name.c_str()))
			{
				Log::e("Mesh '%s' does not exist.", mesh_name.c_str());
				return 0;
			}

			ResourceId id;
			id.id = hash::murmur2_64(mesh_name.c_str(), string::strlen(mesh_name.c_str()), 0);
			m_mesh.push_back(id);
		}
	}

	// Check for meshes
	if (root.has_key("sprite"))
	{
		JSONElement sprite_array = root.key("sprite");
		uint32_t sprite_array_size = sprite_array.size();

		for (uint32_t i = 0; i < sprite_array_size; i++)
		{
			TempAllocator256 alloc;
			DynamicString sprite_name(alloc);
			sprite_name += sprite_array[i].string_value();
			sprite_name += ".sprite";

			if (!fs.is_file(sprite_name.c_str()))
			{
				Log::e("Sprite '%s' does not exist.", sprite_name.c_str());
				return 0;
			}

			ResourceId id;
			id.id = hash::murmur2_64(sprite_name.c_str(), string::strlen(sprite_name.c_str()), 0);
			m_sprite.push_back(id);
		}
	}

	return sizeof(PackageHeader) +
			m_texture.size() * sizeof(ResourceId) +
			m_script.size() * sizeof(ResourceId) +
			m_sound.size() * sizeof(ResourceId) +
			m_mesh.size() * sizeof(ResourceId) +
			m_sprite.size() * sizeof(ResourceId);
}

//-----------------------------------------------------------------------------
void PackageCompiler::write_impl(File* out_file)
{
	PackageHeader header;
	header.num_textures = m_texture.size();
	header.num_scripts = m_script.size();
	header.num_sounds = m_sound.size();
	header.num_meshes = m_mesh.size();
	header.num_sprites = m_sprite.size();

	header.textures_offset = sizeof(PackageHeader);
	header.scripts_offset  = header.textures_offset + sizeof(ResourceId) * header.num_textures;
	header.sounds_offset = header.scripts_offset + sizeof(ResourceId) * header.num_scripts;
	header.meshes_offset = header.sounds_offset + sizeof(ResourceId) * header.num_sounds;
	header.sprites_offset = header.meshes_offset + sizeof(ResourceId) * header.num_meshes;

	out_file->write((char*) &header, sizeof(PackageHeader));

	if (m_texture.size() > 0)
	{
		out_file->write((char*) m_texture.begin(), sizeof(ResourceId) * header.num_textures);		
	}
	if (m_script.size() > 0)
	{
		out_file->write((char*) m_script.begin(), sizeof(ResourceId) * header.num_scripts);
	}
	if (m_sound.size() > 0)
	{
		out_file->write((char*) m_sound.begin(), sizeof(ResourceId) * header.num_sounds);
	}
	if (m_mesh.size() > 0)
	{
		out_file->write((char*) m_mesh.begin(), sizeof(ResourceId) * header.num_meshes);
	}
	if (m_sprite.size() > 0)
	{
		out_file->write((char*) m_sprite.begin(), sizeof(ResourceId) * header.num_sprites);
	}

	// Cleanup
	m_texture.clear();
	m_script.clear();
	m_sound.clear();
	m_mesh.clear();
	m_sprite.clear();
}

} // namespace crown
