/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/globals.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/level_resource.h"
#include "resource/unit_compiler.h"

namespace crown
{
namespace level_resource
{
	const StringId32* unit_names(const LevelResource* lr)
	{
		return (const StringId32*)((char*)lr + lr->unit_names_offset);
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

#if CROWN_CAN_COMPILE
namespace level_resource_internal
{
	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, buf);

		Array<LevelSound> sounds(default_allocator());
		{
			JsonArray sounds_json(ta);
			sjson::parse_array(sounds_json, obj["sounds"]);

			for (u32 i = 0; i < array::size(sounds_json); ++i)
			{
				JsonObject sound(ta);
				sjson::parse_object(sound, sounds_json[i]);

				DynamicString sound_name(ta);
				sjson::parse_string(sound_name, sound["name"]);
				DATA_COMPILER_ASSERT_RESOURCE_EXISTS("sound"
					, sound_name.c_str()
					, opts
					);
				opts.add_requirement("sound", sound_name.c_str());

				LevelSound ls;
				ls.name     = sjson::parse_resource_name(sound["name"]);
				ls.position = sjson::parse_vector3      (sound["position"]);
				ls.volume   = sjson::parse_float        (sound["volume"]);
				ls.range    = sjson::parse_float        (sound["range"]);
				ls.loop     = sjson::parse_bool         (sound["loop"]);

				array::push_back(sounds, ls);
			}
		}

		UnitCompiler uc(opts);
		s32 err = 0;
		err = uc.compile_units_array(obj["units"], UINT32_MAX);
		DATA_COMPILER_ENSURE(err == 0, opts);

		Buffer unit_blob = uc.blob();

		// Write
		LevelResource lr;
		lr.version           = RESOURCE_HEADER(RESOURCE_VERSION_LEVEL);
		lr.num_units         = uc._num_units;
		lr.num_sounds        = array::size(sounds);
		lr.sounds_offset     = sizeof(lr);
		lr.unit_names_offset = lr.sounds_offset + sizeof(LevelSound) * lr.num_sounds;
		lr.units_offset      = lr.unit_names_offset + sizeof(StringId32) * lr.num_units;
		lr.units_offset      = (u32)(uintptr_t)memory::align_top((void*)(uintptr_t)lr.units_offset, 16);

		opts.write(lr.version);
		opts.write(lr.num_units);
		opts.write(lr.unit_names_offset);
		opts.write(lr.units_offset);
		opts.write(lr.num_sounds);
		opts.write(lr.sounds_offset);

		// Write level sounds
		for (u32 i = 0; i < array::size(sounds); ++i)
		{
			opts.write(sounds[i].name);
			opts.write(sounds[i].position);
			opts.write(sounds[i].volume);
			opts.write(sounds[i].range);
			opts.write(sounds[i].loop);
		}

		// Write unit names
		for (u32 i = 0; i < array::size(uc._unit_names); ++i)
			opts.write(uc._unit_names[i]._id);

		// Write units
		opts.align(16);
		opts.write(unit_blob);
		return 0;
	}

} // namespace level_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
