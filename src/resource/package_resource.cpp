/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/map.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_id.h"
#include "resource/compile_options.h"
#include "resource/package_resource.h"

namespace crown
{
namespace package_resource_internal
{
	void compile_resources(const char* type, const JsonArray& names, Array<PackageResource::Resource>& output, CompileOptions& opts)
	{
		const StringId64 typeh = StringId64(type);

		for (u32 i = 0; i < array::size(names); ++i)
		{
			TempAllocator1024 ta;
			DynamicString name(ta);
			sjson::parse_string(names[i], name);

			DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name.c_str(), opts);

			const StringId64 nameh = sjson::parse_resource_id(names[i]);
			array::push_back(output, PackageResource::Resource(typeh, nameh));
		}
	}

	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

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

		if (json_object::has(object, "texture"))          sjson::parse_array(object["texture"], texture);
		if (json_object::has(object, "lua"))              sjson::parse_array(object["lua"], script);
		if (json_object::has(object, "sound"))            sjson::parse_array(object["sound"], sound);
		if (json_object::has(object, "mesh"))             sjson::parse_array(object["mesh"], mesh);
		if (json_object::has(object, "unit"))             sjson::parse_array(object["unit"], unit);
		if (json_object::has(object, "sprite"))           sjson::parse_array(object["sprite"], sprite);
		if (json_object::has(object, "material"))         sjson::parse_array(object["material"], material);
		if (json_object::has(object, "font"))             sjson::parse_array(object["font"], font);
		if (json_object::has(object, "level"))            sjson::parse_array(object["level"], level);
		if (json_object::has(object, "physics_config"))   sjson::parse_array(object["physics_config"], phyconf);
		if (json_object::has(object, "shader"))           sjson::parse_array(object["shader"], shader);
		if (json_object::has(object, "sprite_animation")) sjson::parse_array(object["sprite_animation"], sprite_animation);

		Array<PackageResource::Resource> resources(default_allocator());

		compile_resources("texture", texture, resources, opts);
		compile_resources("lua", script, resources, opts);
		compile_resources("sound", sound, resources, opts);
		compile_resources("mesh", mesh, resources, opts);
		compile_resources("unit", unit, resources, opts);
		compile_resources("sprite", sprite, resources, opts);
		compile_resources("material", material, resources, opts);
		compile_resources("font", font, resources, opts);
		compile_resources("level", level, resources, opts);
		compile_resources("physics_config", phyconf, resources, opts);
		compile_resources("shader", shader, resources, opts);
		compile_resources("sprite_animation", sprite_animation, resources, opts);

		// Write
		opts.write(RESOURCE_VERSION_PACKAGE);
		opts.write(array::size(resources));

		for (u32 i = 0; i < array::size(resources); ++i)
		{
			opts.write(resources[i].type);
			opts.write(resources[i].name);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);
		CE_ASSERT(version == RESOURCE_VERSION_PACKAGE, "Wrong version");

		u32 num_resources;
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

} // namespace package_resource_internal

} // namespace crown
