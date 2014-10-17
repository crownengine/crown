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

#include "file.h"
#include "filesystem.h"
#include "json_parser.h"
#include "package_resource.h"
#include "temp_allocator.h"
#include "reader_writer.h"

namespace crown
{
namespace package_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		JSONElement texture  = root.key_or_nil("texture");
		JSONElement script   = root.key_or_nil("lua");
		JSONElement sound    = root.key_or_nil("sound");
		JSONElement mesh     = root.key_or_nil("mesh");
		JSONElement unit     = root.key_or_nil("unit");
		JSONElement sprite   = root.key_or_nil("sprite");
		JSONElement physics  = root.key_or_nil("physics");
		JSONElement material = root.key_or_nil("material");
		JSONElement font     = root.key_or_nil("font");
		JSONElement level    = root.key_or_nil("level");
		JSONElement phyconf  = root.key_or_nil("physics_config");
		JSONElement shader   = root.key_or_nil("shader");
		JSONElement sprite_animation = root.key_or_nil("sprite_animation");

		const uint32_t num_textures  = texture.is_nil() ? 0 : texture.size();
		const uint32_t num_scripts   = script.is_nil() ? 0 : script.size();
		const uint32_t num_sounds    = sound.is_nil() ? 0 : sound.size();
		const uint32_t num_meshes    = mesh.is_nil() ? 0 : mesh.size();
		const uint32_t num_units     = unit.is_nil() ? 0 : unit.size();
		const uint32_t num_sprites   = sprite.is_nil() ? 0 : sprite.size();
		const uint32_t num_physics   = physics.is_nil() ? 0 : physics.size();
		const uint32_t num_materials = material.is_nil() ? 0 : material.size();
		const uint32_t num_fonts     = font.is_nil() ? 0 : font.size();
		const uint32_t num_levels    = level.is_nil() ? 0 : level.size();
		const uint32_t num_phyconfs  = phyconf.is_nil() ? 0 : phyconf.size();
		const uint32_t num_shaders   = shader.is_nil() ? 0 : shader.size();
		const uint32_t num_sprite_animations = sprite_animation.is_nil() ? 0 : sprite_animation.size();

		// Write header
		opts.write(VERSION);
		opts.write(num_textures);
		uint32_t offt = sizeof(PackageResource);
		opts.write(offt);

		opts.write(num_scripts);
		offt += sizeof(StringId64) * num_textures;
		opts.write(offt);

		opts.write(num_sounds);
		offt += sizeof(StringId64) * num_scripts;
		opts.write(offt);

		opts.write(num_meshes);
		offt += sizeof(StringId64) * num_sounds;
		opts.write(offt);

		opts.write(num_units);
		offt += sizeof(StringId64) * num_meshes;
		opts.write(offt);

		opts.write(num_sprites);
		offt += sizeof(StringId64) * num_units;
		opts.write(offt);

		opts.write(num_physics);
		offt += sizeof(StringId64) * num_sprites;
		opts.write(offt);

		opts.write(num_materials);
		offt += sizeof(StringId64) * num_physics;
		opts.write(offt);

		opts.write(num_fonts);
		offt += sizeof(StringId64) * num_materials;
		opts.write(offt);

		opts.write(num_levels);
		offt += sizeof(StringId64) * num_fonts;
		opts.write(offt);

		opts.write(num_phyconfs);
		offt += sizeof(StringId64) * num_levels;
		opts.write(offt);

		opts.write(num_shaders);
		offt += sizeof(StringId64) * num_phyconfs;
		opts.write(offt);

		opts.write(num_sprite_animations);
		offt += sizeof(StringId64) * num_shaders;
		opts.write(offt);

		// Write resource ids
		for (uint32_t i = 0; i < num_textures; i++)
			opts.write(texture[i].to_resource_id("texture").name);

		for (uint32_t i = 0; i < num_scripts; i++)
			opts.write(script[i].to_resource_id("lua").name);

		for (uint32_t i = 0; i < num_sounds; i++)
			opts.write(sound[i].to_resource_id("sound").name);

		for (uint32_t i = 0; i < num_meshes; i++)
			opts.write(mesh[i].to_resource_id("mesh").name);

		for (uint32_t i = 0; i < num_units; i++)
			opts.write(unit[i].to_resource_id("unit").name);

		for (uint32_t i = 0; i < num_sprites; i++)
			opts.write(sprite[i].to_resource_id("sprite").name);

		for (uint32_t i = 0; i < num_physics; i++)
			opts.write(physics[i].to_resource_id("physics").name);

		for (uint32_t i = 0; i < num_materials; i++)
			opts.write(material[i].to_resource_id("material").name);

		for (uint32_t i = 0; i < num_fonts; i++)
			opts.write(font[i].to_resource_id("font").name);

		for (uint32_t i = 0; i < num_levels; i++)
			opts.write(level[i].to_resource_id("level").name);

		for (uint32_t i = 0; i < num_phyconfs; i++)
			opts.write(phyconf[i].to_resource_id("physics_config").name);

		for (uint32_t i = 0; i < num_shaders; i++)
			opts.write(shader[i].to_resource_id("shader").name);

		for (uint32_t i = 0; i < num_sprite_animations; i++)
			opts.write(sprite_animation[i].to_resource_id("sprite_animation").name);
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t num_textures(const PackageResource* pr)
	{
		return pr->num_textures;
	}

	uint32_t num_scripts(const PackageResource* pr)
	{
		return pr->num_scripts;
	}

	uint32_t num_sounds(const PackageResource* pr)
	{
		return pr->num_sounds;
	}

	uint32_t num_meshes(const PackageResource* pr)
	{
		return pr->num_meshes;
	}

	uint32_t num_units(const PackageResource* pr)
	{
		return pr->num_units;
	}

	uint32_t num_sprites(const PackageResource* pr)
	{
		return pr->num_sprites;
	}

	uint32_t num_physics(const PackageResource* pr)
	{
		return pr->num_physics;
	}

	uint32_t num_materials(const PackageResource* pr)
	{
		return pr->num_materials;
	}

	uint32_t num_fonts(const PackageResource* pr)
	{
		return pr->num_fonts;
	}

	uint32_t num_levels(const PackageResource* pr)
	{
		return pr->num_levels;
	}

	uint32_t num_physics_configs(const PackageResource* pr)
	{
		return pr->num_physics_configs;
	}

	uint32_t num_shaders(const PackageResource* pr)
	{
		return pr->num_shaders;
	}

	uint32_t num_sprite_animations(const PackageResource* pr)
	{
		return pr->num_sprite_animations;
	}

	StringId64 get_texture_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_textures(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->textures_offset);
		return begin[i];
	}

	StringId64 get_script_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_scripts(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->scripts_offset);
		return begin[i];
	}

	StringId64 get_sound_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_sounds(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->sounds_offset);
		return begin[i];
	}

	StringId64 get_mesh_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_meshes(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->meshes_offset);
		return begin[i];
	}

	StringId64 get_unit_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_units(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->units_offset);
		return begin[i];
	}

	StringId64 get_sprite_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_sprites(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->sprites_offset);
		return begin[i];
	}

	StringId64 get_physics_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_physics(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->physics_offset);
		return begin[i];
	}

	StringId64 get_material_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_materials(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->materials_offset);
		return begin[i];
	}

	StringId64 get_font_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_fonts(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->fonts_offset);
		return begin[i];
	}

	StringId64 get_level_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_levels(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->levels_offset);
		return begin[i];
	}

	StringId64 get_physics_config_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_physics_configs(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->physics_configs_offset);
		return begin[i];
	}

	StringId64 get_shader_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_shaders(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->shaders_offset);
		return begin[i];
	}

	StringId64 get_sprite_animation_id(const PackageResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_sprite_animations(pr), "Index out of bounds");
		StringId64* begin = (StringId64*) ((char*)pr + pr->sprite_animations_offset);
		return begin[i];
	}
} // namespace package_resource
} // namespace crown
