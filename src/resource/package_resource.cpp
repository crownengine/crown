/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "file.h"
#include "filesystem.h"
#include "package_resource.h"
#include "temp_allocator.h"
#include "reader_writer.h"
#include "compile_options.h"
#include "array.h"
#include "sjson.h"
#include "map.h"

namespace crown
{
namespace package_resource
{
	void compile_resources(const char* type, const JsonArray& names, Array<PackageResource::Resource>& output, CompileOptions& opts)
	{
		const StringId64 typeh = StringId64(type);

		for (u32 i = 0; i < array::size(names); ++i)
		{
			TempAllocator1024 ta;
			DynamicString name(ta);
			sjson::parse_string(names[i], name);

			RESOURCE_COMPILER_ASSERT(opts.resource_exists(type, name.c_str())
				, opts
				, "Resource does not exist: '%s.%s'"
				, name.c_str()
				, type
				);

			const StringId64 nameh = sjson::parse_resource_id(names[i]);
			array::push_back(output, PackageResource::Resource(typeh, nameh));
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

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

		if (map::has(object, FixedString("texture")))          sjson::parse_array(object["texture"], texture);
		if (map::has(object, FixedString("lua")))              sjson::parse_array(object["lua"], script);
		if (map::has(object, FixedString("sound")))            sjson::parse_array(object["sound"], sound);
		if (map::has(object, FixedString("mesh")))             sjson::parse_array(object["mesh"], mesh);
		if (map::has(object, FixedString("unit")))             sjson::parse_array(object["unit"], unit);
		if (map::has(object, FixedString("sprite")))           sjson::parse_array(object["sprite"], sprite);
		if (map::has(object, FixedString("material")))         sjson::parse_array(object["material"], material);
		if (map::has(object, FixedString("font")))             sjson::parse_array(object["font"], font);
		if (map::has(object, FixedString("level")))            sjson::parse_array(object["level"], level);
		if (map::has(object, FixedString("physics_config")))   sjson::parse_array(object["physics_config"], phyconf);
		if (map::has(object, FixedString("shader")))           sjson::parse_array(object["shader"], shader);
		if (map::has(object, FixedString("sprite_animation"))) sjson::parse_array(object["sprite_animation"], sprite_animation);

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
		opts.write(PACKAGE_VERSION);
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
		CE_ASSERT(version == PACKAGE_VERSION, "Wrong version");

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
} // namespace package_resource
} // namespace crown
