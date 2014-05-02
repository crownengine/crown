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
#include "StringUtils.h"
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

	Array<ResourceId> m_texture(default_allocator());
	Array<ResourceId> m_script(default_allocator());
	Array<ResourceId> m_sound(default_allocator());
	Array<ResourceId> m_mesh(default_allocator());
	Array<ResourceId> m_unit(default_allocator());
	Array<ResourceId> m_sprite(default_allocator());
	Array<ResourceId> m_physics(default_allocator());
	Array<ResourceId> m_materials(default_allocator());
	Array<ResourceId> m_guis(default_allocator());
	Array<ResourceId> m_fonts(default_allocator());
	Array<ResourceId> m_levels(default_allocator());

	// Check for resource types
	if (root.has_key("texture"))
	{
		JSONElement texture_array = root.key("texture");
		uint32_t texture_array_size = texture_array.size();

		for (uint32_t i = 0; i < texture_array_size; i++)
		{
			DynamicString texture_name;
			texture_array[i].to_string(texture_name); texture_name += ".texture";

			if (!fs.is_file(texture_name.c_str()))
			{
				Log::e("Texture '%s' does not exist.", texture_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(texture_name.c_str(), texture_name.length(), 0);
			array::push_back(m_texture, id);
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
			lua_array[i].to_string(lua_name); lua_name += ".lua";

			if (!fs.is_file(lua_name.c_str()))
			{
				Log::e("Lua script '%s' does not exist.", lua_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(lua_name.c_str(), lua_name.length(), 0);
			array::push_back(m_script, id);
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
			sound_array[i].to_string(sound_name); sound_name += ".sound";

			if (!fs.is_file(sound_name.c_str()))
			{
				Log::e("Sound '%s' does not exist.", sound_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(sound_name.c_str(), string::strlen(sound_name.c_str()), 0);
			array::push_back(m_sound, id);
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
			mesh_array[i].to_string(mesh_name); mesh_name += ".mesh";

			if (!fs.is_file(mesh_name.c_str()))
			{
				Log::e("Mesh '%s' does not exist.", mesh_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(mesh_name.c_str(), mesh_name.length(), 0);
			array::push_back(m_mesh, id);
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
			unit_array[i].to_string(unit_name); unit_name += ".unit";

			if (!fs.is_file(unit_name.c_str()))
			{
				Log::e("Unit '%s' does not exist.", unit_name.c_str());
			}

			ResourceId id;
			id.id = string::murmur2_64(unit_name.c_str(), unit_name.length(), 0);
			array::push_back(m_unit, id);
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
			sprite_array[i].to_string(sprite_name); sprite_name += ".sprite";

			if (!fs.is_file(sprite_name.c_str()))
			{
				Log::e("Sprite '%s' does not exist.", sprite_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(sprite_name.c_str(), sprite_name.length(), 0);
			array::push_back(m_sprite, id);
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
			physics_array[i].to_string(physics_name); physics_name += ".physics";

			if (!fs.is_file(physics_name.c_str()))
			{
				Log::e("Physics '%s' does not exist.", physics_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(physics_name.c_str(), physics_name.length(), 0);
			array::push_back(m_physics, id);
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
			materials_array[i].to_string(materials_name); materials_name += ".material";

			if (!fs.is_file(materials_name.c_str()))
			{
				Log::e("Material '%s' does not exist.", materials_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(materials_name.c_str(), materials_name.length(), 0);
			array::push_back(m_materials, id);
		}
	}

	// Check for materials
	if (root.has_key("gui"))
	{
		JSONElement guis_array = root.key("gui");
		uint32_t guis_array_size = guis_array.size();

		for (uint32_t i = 0; i < guis_array_size; i++)
		{
			DynamicString guis_name;
			guis_array[i].to_string(guis_name); guis_name += ".gui";

			if (!fs.is_file(guis_name.c_str()))
			{
				Log::e("gui '%s' does not exist.", guis_name.c_str());
				return;
			}

			ResourceId id;
			id.id = string::murmur2_64(guis_name.c_str(), guis_name.length(), 0);
			array::push_back(m_guis, id);
		}
	}

	// Check for fonts
	if (root.has_key("font"))
	{
		JSONElement fonts_array = root.key("font");
		uint32_t fonts_array_size = fonts_array.size();

		for (uint32_t i = 0; i < fonts_array_size; i++)
		{
			DynamicString font_name;
			fonts_array[i].to_string(font_name); font_name += ".font";

			if (!fs.is_file(font_name.c_str()))
			{
				Log::e("font '%s' does not exist.", font_name.c_str());
				return;				
			}

			ResourceId id;
			id.id = string::murmur2_64(font_name.c_str(), font_name.length(), 0);
			array::push_back(m_fonts, id);
		}
	}

	// Check for fonts
	if (root.has_key("level"))
	{
		JSONElement levels_array = root.key("level");
		uint32_t levels_array_size = levels_array.size();

		for (uint32_t i = 0; i < levels_array_size; i++)
		{
			DynamicString level_name;
			levels_array[i].to_string(level_name); level_name += ".level";

			if (!fs.is_file(level_name.c_str()))
			{
				Log::e("level '%s' does not exist.", level_name.c_str());
				return;				
			}

			ResourceId id;
			id.id = string::murmur2_64(level_name.c_str(), level_name.length(), 0);
			array::push_back(m_levels, id);
		}
	}

	PackageHeader header;
	header.num_textures = array::size(m_texture);
	header.num_scripts = array::size(m_script);
	header.num_sounds = array::size(m_sound);
	header.num_meshes = array::size(m_mesh);
	header.num_units = array::size(m_unit);
	header.num_sprites = array::size(m_sprite);
	header.num_physics = array::size(m_physics);
	header.num_materials = array::size(m_materials);
	header.num_guis = array::size(m_guis);
	header.num_fonts = array::size(m_fonts);
	header.num_levels = array::size(m_levels);

	header.textures_offset = sizeof(PackageHeader);
	header.scripts_offset  = header.textures_offset + sizeof(ResourceId) * header.num_textures;
	header.sounds_offset = header.scripts_offset + sizeof(ResourceId) * header.num_scripts;
	header.meshes_offset = header.sounds_offset + sizeof(ResourceId) * header.num_sounds;
	header.units_offset = header.meshes_offset + sizeof(ResourceId) * header.num_meshes;
	header.sprites_offset = header.units_offset + sizeof(ResourceId) * header.num_units;
	header.physics_offset = header.sprites_offset + sizeof(ResourceId) * header.num_sprites;
	header.materials_offset = header.physics_offset + sizeof(ResourceId) * header.num_physics;
	header.guis_offset = header.materials_offset + sizeof(ResourceId) * header.num_materials;
	header.fonts_offset = header.guis_offset + sizeof(ResourceId) * header.num_guis;
	header.levels_offset = header.fonts_offset + sizeof(ResourceId) * header.num_fonts;

	out_file->write((char*) &header, sizeof(PackageHeader));

	if (array::size(m_texture) > 0)
	{
		out_file->write((char*) array::begin(m_texture), sizeof(ResourceId) * header.num_textures);		
	}
	if (array::size(m_script) > 0)
	{
		out_file->write((char*) array::begin(m_script), sizeof(ResourceId) * header.num_scripts);
	}
	if (array::size(m_sound) > 0)
	{
		out_file->write((char*) array::begin(m_sound), sizeof(ResourceId) * header.num_sounds);
	}
	if (array::size(m_mesh) > 0)
	{
		out_file->write((char*) array::begin(m_mesh), sizeof(ResourceId) * header.num_meshes);
	}
	if (array::size(m_unit) > 0)
	{
		out_file->write((char*) array::begin(m_unit), sizeof(ResourceId) * header.num_units);	
	}
	if (array::size(m_sprite) > 0)
	{
		out_file->write((char*) array::begin(m_sprite), sizeof(ResourceId) * header.num_sprites);
	}
	if (array::size(m_physics) > 0)
	{
		out_file->write((char*) array::begin(m_physics), sizeof(ResourceId) * header.num_physics);
	}
	if (array::size(m_materials) > 0)
	{
		out_file->write((char*) array::begin(m_materials), sizeof(ResourceId) * header.num_materials);
	}
	if (array::size(m_guis) > 0)
	{
		out_file->write((char*) array::begin(m_guis), sizeof(ResourceId) * header.num_guis);
	}
	if (array::size(m_fonts) > 0)
	{
		out_file->write((char*) array::begin(m_fonts), sizeof(ResourceId) * header.num_fonts);
	}
	if (array::size(m_levels) > 0)
	{
		out_file->write((char*) array::begin(m_levels), sizeof(ResourceId) * header.num_levels);
	}
}

} // namespace package_resource
} // namespace crown
