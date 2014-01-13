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
#include "Log.h"
#include "PackageResource.h"
#include "TempAllocator.h"

namespace crown
{
namespace package_resource
{

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);

	char file_buf[4096];
	file->read(file_buf, file->size());
	fs.close(file);

	JSONParser json(file_buf);
	JSONElement root = json.root();

	List<ResourceId> m_texture(default_allocator());
	List<ResourceId> m_script(default_allocator());
	List<ResourceId> m_sound(default_allocator());
	List<ResourceId> m_mesh(default_allocator());
	List<ResourceId> m_unit(default_allocator());
	List<ResourceId> m_sprite(default_allocator());
	List<ResourceId> m_physics(default_allocator());
	List<ResourceId> m_materials(default_allocator());

	// Check for resource types
	if (root.has_key("texture"))
	{
		JSONElement texture_array = root.key("texture");
		uint32_t texture_array_size = texture_array.size();

		for (uint32_t i = 0; i < texture_array_size; i++)
		{
			DynamicString texture_name;
			texture_array[i].string_value(texture_name); texture_name += ".texture";
			Log::d("texture name = %s", texture_name.c_str());

			if (!fs.is_file(texture_name.c_str()))
			{
				Log::e("Texture '%s' does not exist.", texture_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(texture_name.c_str(), texture_name.length(), 0);
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
			DynamicString lua_name;
			lua_array[i].string_value(lua_name); lua_name += ".lua";

			if (!fs.is_file(lua_name.c_str()))
			{
				Log::e("Lua script '%s' does not exist.", lua_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(lua_name.c_str(), lua_name.length(), 0);
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
			DynamicString sound_name;
			sound_array[i].string_value(sound_name); sound_name += ".sound";

			if (!fs.is_file(sound_name.c_str()))
			{
				Log::e("Sound '%s' does not exist.", sound_name.c_str());
				return;
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
			DynamicString mesh_name;
			mesh_array[i].string_value(mesh_name); mesh_name += ".mesh";

			if (!fs.is_file(mesh_name.c_str()))
			{
				Log::e("Mesh '%s' does not exist.", mesh_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(mesh_name.c_str(), mesh_name.length(), 0);
			m_mesh.push_back(id);
		}
	}

	// Check for units
	if (root.has_key("unit"))
	{
		JSONElement unit_array = root.key("unit");
		uint32_t unit_array_size = unit_array.size();

		for (uint32_t i = 0; i < unit_array_size; i++)
		{
			DynamicString unit_name;
			unit_array[i].string_value(unit_name); unit_name += ".unit";

			if (!fs.is_file(unit_name.c_str()))
			{
				Log::e("Unit '%s' does not exist.", unit_name.c_str());
			}

			ResourceId id;
			id.id = hash::murmur2_64(unit_name.c_str(), unit_name.length(), 0);
			m_unit.push_back(id);
		}
	}

	// Check for meshes
	if (root.has_key("sprite"))
	{
		JSONElement sprite_array = root.key("sprite");
		uint32_t sprite_array_size = sprite_array.size();

		for (uint32_t i = 0; i < sprite_array_size; i++)
		{
			DynamicString sprite_name;
			sprite_array[i].string_value(sprite_name); sprite_name += ".sprite";

			if (!fs.is_file(sprite_name.c_str()))
			{
				Log::e("Sprite '%s' does not exist.", sprite_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(sprite_name.c_str(), sprite_name.length(), 0);
			m_sprite.push_back(id);
		}
	}

	// Check for physics
	if (root.has_key("physics"))
	{
		JSONElement physics_array = root.key("physics");
		uint32_t physics_array_size = physics_array.size();

		for (uint32_t i = 0; i < physics_array_size; i++)
		{
			DynamicString physics_name;
			physics_array[i].string_value(physics_name); physics_name += ".physics";

			if (!fs.is_file(physics_name.c_str()))
			{
				Log::e("Physics '%s' does not exist.", physics_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(physics_name.c_str(), physics_name.length(), 0);
			m_physics.push_back(id);
		}	
	}

	// Check for materials
	if (root.has_key("material"))
	{
		JSONElement materials_array = root.key("material");
		uint32_t materials_array_size = materials_array.size();

		for (uint32_t i = 0; i < materials_array_size; i++)
		{
			DynamicString materials_name;
			materials_array[i].string_value(materials_name); materials_name += ".material";

			if (!fs.is_file(materials_name.c_str()))
			{
				Log::e("Material '%s' does not exist.", materials_name.c_str());
				return;
			}

			ResourceId id;
			id.id = hash::murmur2_64(materials_name.c_str(), materials_name.length(), 0);
			m_materials.push_back(id);
		}
	}

	PackageHeader header;
	header.num_textures = m_texture.size();
	header.num_scripts = m_script.size();
	header.num_sounds = m_sound.size();
	header.num_meshes = m_mesh.size();
	header.num_units = m_unit.size();
	header.num_sprites = m_sprite.size();
	header.num_physics = m_physics.size();
	header.num_materials = m_materials.size();

	header.textures_offset = sizeof(PackageHeader);
	header.scripts_offset  = header.textures_offset + sizeof(ResourceId) * header.num_textures;
	header.sounds_offset = header.scripts_offset + sizeof(ResourceId) * header.num_scripts;
	header.meshes_offset = header.sounds_offset + sizeof(ResourceId) * header.num_sounds;
	header.units_offset = header.meshes_offset + sizeof(ResourceId) * header.num_meshes;
	header.sprites_offset = header.units_offset + sizeof(ResourceId) * header.num_units;
	header.physics_offset = header.sprites_offset + sizeof(ResourceId) * header.num_sprites;
	header.materials_offset = header.physics_offset + sizeof(ResourceId) * header.num_physics;

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
	if (m_unit.size() > 0)
	{
		out_file->write((char*) m_unit.begin(), sizeof(ResourceId) * header.num_units);	
	}
	if (m_sprite.size() > 0)
	{
		out_file->write((char*) m_sprite.begin(), sizeof(ResourceId) * header.num_sprites);
	}
	if (m_physics.size() > 0)
	{
		out_file->write((char*) m_physics.begin(), sizeof(ResourceId) * header.num_physics);
	}
	if (m_materials.size() > 0)
	{
		out_file->write((char*) m_materials.begin(), sizeof(ResourceId) * header.num_materials);
	}
}

} // namespace package_resource
} // namespace crown
