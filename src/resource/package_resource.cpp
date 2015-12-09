/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "file.h"
#include "filesystem.h"
#include "json_parser.h"
#include "package_resource.h"
#include "temp_allocator.h"
#include "reader_writer.h"
#include "compile_options.h"
#include "array.h"

namespace crown
{
namespace package_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		JSONElement texture  = root.key_or_nil("texture");
		JSONElement script   = root.key_or_nil("lua");
		JSONElement sound    = root.key_or_nil("sound");
		JSONElement mesh     = root.key_or_nil("mesh");
		JSONElement unit     = root.key_or_nil("unit");
		JSONElement sprite   = root.key_or_nil("sprite");
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
		const uint32_t num_materials = material.is_nil() ? 0 : material.size();
		const uint32_t num_fonts     = font.is_nil() ? 0 : font.size();
		const uint32_t num_levels    = level.is_nil() ? 0 : level.size();
		const uint32_t num_phyconfs  = phyconf.is_nil() ? 0 : phyconf.size();
		const uint32_t num_shaders   = shader.is_nil() ? 0 : shader.size();
		const uint32_t num_sprite_animations = sprite_animation.is_nil() ? 0 : sprite_animation.size();

		Array<PackageResource::Resource> resources(default_allocator());

		for (uint32_t i = 0; i < num_textures; ++i)
		{
			PackageResource::Resource res(TEXTURE_TYPE, texture[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_scripts; ++i)
		{
			PackageResource::Resource res(SCRIPT_TYPE, script[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sounds; ++i)
		{
			PackageResource::Resource res(SOUND_TYPE, sound[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_meshes; ++i)
		{
			PackageResource::Resource res(MESH_TYPE, mesh[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_units; ++i)
		{
			PackageResource::Resource res(UNIT_TYPE, unit[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sprites; ++i)
		{
			PackageResource::Resource res(SPRITE_TYPE, sprite[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_physics; ++i)
		{
			PackageResource::Resource res(PHYSICS_TYPE, physics[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_materials; ++i)
		{
			PackageResource::Resource res(MATERIAL_TYPE, material[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_fonts; ++i)
		{
			PackageResource::Resource res(FONT_TYPE, font[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_levels; ++i)
		{
			PackageResource::Resource res(LEVEL_TYPE, level[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_phyconfs; ++i)
		{
			PackageResource::Resource res(PHYSICS_CONFIG_TYPE, phyconf[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_shaders; ++i)
		{
			PackageResource::Resource res(SHADER_TYPE, shader[i].to_resource_id());
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sprite_animations; ++i)
		{
			PackageResource::Resource res(SPRITE_ANIMATION_TYPE, sprite_animation[i].to_resource_id());
			array::push_back(resources, res);
		}

		// Write header
		opts.write(PACKAGE_VERSION);
		opts.write(array::size(resources));

		// Write resource ids
		for (uint32_t i = 0; i < array::size(resources); ++i)
		{
			opts.write(resources[i].type);
			opts.write(resources[i].name);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		uint32_t version;
		br.read(version);
		CE_ASSERT(version == PACKAGE_VERSION, "Wrong version");

		uint32_t num_resources;
		br.read(num_resources);

		PackageResource* pr = CE_NEW(a, PackageResource)(a);
		array::resize(pr->resources, num_resources);
		br.read(array::begin(pr->resources), sizeof(PackageResource::Resource)*num_resources);

		return pr;
	}

	void unload(Allocator& a, void* resource)
	{
		CE_DELETE(a, (PackageResource*)resource);
	}
} // namespace package_resource
} // namespace crown
