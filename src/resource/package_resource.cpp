/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "file.h"
#include "filesystem.h"
#include "package_resource.h"
#include "temp_allocator.h"
#include "reader_writer.h"
#include "compile_options.h"
#include "array.h"
#include "njson.h"
#include "map.h"

namespace crown
{
namespace package_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		njson::parse(buf, object);

		JsonArray texture(ta);
		JsonArray script(ta);
		JsonArray sound(ta);
		JsonArray mesh(ta);
		JsonArray unit(ta);
		JsonArray sprite(ta);
		JsonArray material(ta);
		JsonArray font(ta);
		JsonArray level(ta);
		JsonArray phyconf(ta);
		JsonArray shader(ta);
		JsonArray sprite_animation(ta);

		if (map::has(object, FixedString("texture")))          njson::parse_array(object["texture"], texture);
		if (map::has(object, FixedString("lua")))              njson::parse_array(object["lua"], script);
		if (map::has(object, FixedString("sound")))            njson::parse_array(object["sound"], sound);
		if (map::has(object, FixedString("mesh")))             njson::parse_array(object["mesh"], mesh);
		if (map::has(object, FixedString("unit")))             njson::parse_array(object["unit"], unit);
		if (map::has(object, FixedString("sprite")))           njson::parse_array(object["sprite"], sprite);
		if (map::has(object, FixedString("material")))         njson::parse_array(object["material"], material);
		if (map::has(object, FixedString("font")))             njson::parse_array(object["font"], font);
		if (map::has(object, FixedString("level")))            njson::parse_array(object["level"], level);
		if (map::has(object, FixedString("physics_config")))   njson::parse_array(object["physics_config"], phyconf);
		if (map::has(object, FixedString("shader")))           njson::parse_array(object["shader"], shader);
		if (map::has(object, FixedString("sprite_animation"))) njson::parse_array(object["sprite_animation"], sprite_animation);

		const uint32_t num_textures    = array::size(texture);
		const uint32_t num_scripts     = array::size(script);
		const uint32_t num_sounds      = array::size(sound);
		const uint32_t num_meshes      = array::size(mesh);
		const uint32_t num_units       = array::size(unit);
		const uint32_t num_sprites     = array::size(sprite);
		const uint32_t num_materials   = array::size(material);
		const uint32_t num_fonts       = array::size(font);
		const uint32_t num_levels      = array::size(level);
		const uint32_t num_phyconfs    = array::size(phyconf);
		const uint32_t num_shaders     = array::size(shader);
		const uint32_t num_sanimations = array::size(sprite_animation);

		Array<PackageResource::Resource> resources(default_allocator());

		for (uint32_t i = 0; i < num_textures; ++i)
		{
			PackageResource::Resource res(TEXTURE_TYPE, njson::parse_resource_id(texture[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_scripts; ++i)
		{
			PackageResource::Resource res(SCRIPT_TYPE, njson::parse_resource_id(script[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sounds; ++i)
		{
			PackageResource::Resource res(SOUND_TYPE, njson::parse_resource_id(sound[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_meshes; ++i)
		{
			PackageResource::Resource res(MESH_TYPE, njson::parse_resource_id(mesh[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_units; ++i)
		{
			PackageResource::Resource res(UNIT_TYPE, njson::parse_resource_id(unit[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sprites; ++i)
		{
			PackageResource::Resource res(SPRITE_TYPE, njson::parse_resource_id(sprite[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_materials; ++i)
		{
			PackageResource::Resource res(MATERIAL_TYPE, njson::parse_resource_id(material[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_fonts; ++i)
		{
			PackageResource::Resource res(FONT_TYPE, njson::parse_resource_id(font[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_levels; ++i)
		{
			PackageResource::Resource res(LEVEL_TYPE, njson::parse_resource_id(level[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_phyconfs; ++i)
		{
			PackageResource::Resource res(PHYSICS_CONFIG_TYPE, njson::parse_resource_id(phyconf[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_shaders; ++i)
		{
			PackageResource::Resource res(SHADER_TYPE, njson::parse_resource_id(shader[i]));
			array::push_back(resources, res);
		}

		for (uint32_t i = 0; i < num_sanimations; ++i)
		{
			PackageResource::Resource res(SPRITE_ANIMATION_TYPE, njson::parse_resource_id(sprite_animation[i]));
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
