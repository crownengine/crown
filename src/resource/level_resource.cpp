/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/map.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/memory.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "resource/compile_options.h"
#include "resource/level_resource.h"
#include "resource/unit_compiler.h"

namespace crown
{
namespace level_resource_internal
{
	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		Array<LevelSound> sounds(default_allocator());
		{
			JsonArray sounds_json(ta);
			sjson::parse_array(object["sounds"], sounds_json);

			for (u32 i = 0; i < array::size(sounds_json); ++i)
			{
				JsonObject sound(ta);
				sjson::parse_object(sounds_json[i], sound);

				DynamicString sound_name(ta);
				sjson::parse_string(sound["name"], sound_name);
				DATA_COMPILER_ASSERT_RESOURCE_EXISTS("sound"
					, sound_name.c_str()
					, opts
					);

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

} // namespace level_resource_internal

namespace level_resource
{
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
