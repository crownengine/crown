/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "compile_options.h"
#include "filesystem.h"
#include "json_object.h"
#include "level_resource.h"
#include "map.h"
#include "memory.h"
#include "sjson.h"
#include "unit_compiler.h"

namespace crown
{
namespace level_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		Array<LevelSound> sounds(default_allocator());
		{
			JsonObject sounds_json(ta);
			sjson::parse_object(object["sounds"], sounds_json);

			auto begin = json_object::begin(sounds_json);
			auto end = json_object::end(sounds_json);
			for (; begin != end; ++begin)
			{
				JsonObject sound(ta);
				sjson::parse_object(begin->pair.second, sound);

				LevelSound ls;
				ls.name     = sjson::parse_resource_id(sound["name"]);
				ls.position = sjson::parse_vector3    (sound["position"]);
				ls.volume   = sjson::parse_float      (sound["volume"]);
				ls.range    = sjson::parse_float      (sound["range"]);
				ls.loop     = sjson::parse_bool       (sound["loop"]);

				array::push_back(sounds, ls);
			}
		}

		UnitCompiler uc(opts);
		uc.compile_multiple_units(object["units"]);
		Buffer unit_blob = uc.blob();

		// Write
		LevelResource lr;
		lr.version       = RESOURCE_VERSION_LEVEL;
		lr.num_sounds    = array::size(sounds);
		lr.units_offset  = sizeof(LevelResource);
		lr.sounds_offset = lr.units_offset + array::size(unit_blob);

		opts.write(lr.version);
		opts.write(lr.units_offset);
		opts.write(lr.num_sounds);
		opts.write(lr.sounds_offset);

		opts.write(unit_blob);

		for (u32 i = 0; i < array::size(sounds); ++i)
		{
			opts.write(sounds[i].name);
			opts.write(sounds[i].position);
			opts.write(sounds[i].volume);
			opts.write(sounds[i].range);
			opts.write(sounds[i].loop);
			opts.write(sounds[i]._pad[0]);
			opts.write(sounds[i]._pad[1]);
			opts.write(sounds[i]._pad[2]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const u32 file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(u32*)res == RESOURCE_VERSION_LEVEL, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const UnitResource* unit_resource(const LevelResource* lr)
	{
		return (const UnitResource*)((char*)lr + lr->units_offset);
	}

	u32 num_sounds(const LevelResource* lr)
	{
		return lr->num_sounds;
	}

	const LevelSound* get_sound(const LevelResource* lr, u32 i)
	{
		CE_ASSERT(i < num_sounds(lr), "Index out of bounds");
		const LevelSound* begin = (LevelSound*)((char*)lr + lr->sounds_offset);
		return &begin[i];
	}
} // namespace level_resource
} // namespace crown
