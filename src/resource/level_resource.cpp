/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/globals.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/level_resource.h"
#include "resource/unit_compiler.h"

namespace crown
{
namespace level_resource
{
	const StringId32 *unit_names(const LevelResource *lr)
	{
		return (StringId32 *)((char *)lr + lr->unit_names_offset);
	}

	const UnitResource *unit_resource(const LevelResource *lr)
	{
		return (UnitResource *)((char *)lr + lr->units_offset);
	}

	u32 num_sounds(const LevelResource *lr)
	{
		return lr->num_sounds;
	}

	const LevelSound *get_sound(const LevelResource *lr, u32 i)
	{
		CE_ASSERT(i < num_sounds(lr), "Index out of bounds");
		const LevelSound *begin = (LevelSound *)((char *)lr + lr->sounds_offset);
		return &begin[i];
	}

} // namespace level_resource

#if CROWN_CAN_COMPILE
namespace level_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		Array<LevelSound> sounds(default_allocator());
		if (json_object::has(obj, "sounds")) {
			JsonArray sounds_json(ta);
			RETURN_IF_ERROR(sjson::parse_array(sounds_json, obj["sounds"]), opts);

			for (u32 i = 0; i < array::size(sounds_json); ++i) {
				JsonObject sound(ta);
				RETURN_IF_ERROR(sjson::parse_object(sound, sounds_json[i]), opts);

				DynamicString sound_name(ta);
				RETURN_IF_ERROR(sjson::parse_string(sound_name, sound["name"]), opts);
				RETURN_IF_RESOURCE_MISSING("sound"
					, sound_name.c_str()
					, opts
					);
				opts.add_requirement("sound", sound_name.c_str());

				LevelSound ls;
				ls.name     = RETURN_IF_ERROR(sjson::parse_resource_name(sound["name"]), opts);
				ls.position = RETURN_IF_ERROR(sjson::parse_vector3      (sound["position"]), opts);
				ls.volume   = RETURN_IF_ERROR(sjson::parse_float        (sound["volume"]), opts);
				ls.range    = RETURN_IF_ERROR(sjson::parse_float        (sound["range"]), opts);
				ls.loop     = RETURN_IF_ERROR(sjson::parse_bool         (sound["loop"]), opts);
				ls.group    = StringId32(0u);
				if (json_object::has(sound, "group")) {
					ls.group = RETURN_IF_ERROR(sjson::parse_string_id(sound["group"]), opts);
				}

				array::push_back(sounds, ls);
			}
		}

		UnitCompiler uc(default_allocator());

		if (json_object::has(obj, "units")) {
			s32 err = unit_compiler::parse_unit_array_from_json(uc, obj["units"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		Buffer units_blob(default_allocator());
		s32 err = unit_compiler::blob(units_blob, uc);
		ENSURE_OR_RETURN(err == 0, opts);

		StringId64 skydome_unit;
		if (json_object::has(obj, "skydome_unit")) {
			TempAllocator256 ta;
			DynamicString skydome_unit_name(ta);

			RETURN_IF_ERROR(sjson::parse_string(skydome_unit_name, obj["skydome_unit"]), opts);
			RETURN_IF_RESOURCE_MISSING("unit"
				, skydome_unit_name.c_str()
				, opts
				);
			opts.add_requirement("unit", skydome_unit_name.c_str());
			skydome_unit = RETURN_IF_ERROR(sjson::parse_resource_name(obj["skydome_unit"]), opts);
		} else {
			opts.add_requirement("unit", "core/units/skydome/skydome");
			skydome_unit = STRING_ID_64("core/units/skydome/skydome", UINT64_C(0x60bc932e8c477ec2));
		}

		// Write
		LevelResource lr;
		lr.version           = RESOURCE_HEADER(RESOURCE_VERSION_LEVEL);
		lr.num_units         = uc._num_units;
		lr.num_sounds        = array::size(sounds);
		lr.sounds_offset     = sizeof(lr);
		lr.unit_names_offset = lr.sounds_offset + sizeof(LevelSound) * lr.num_sounds;
		lr.units_offset      = lr.unit_names_offset + sizeof(StringId32) * lr.num_units;
		lr.units_offset      = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)lr.units_offset, 16);
		lr.skydome_unit      = skydome_unit;

		opts.write(lr.version);
		opts.write(lr.num_units);
		opts.write(lr.unit_names_offset);
		opts.write(lr.units_offset);
		opts.write(lr.num_sounds);
		opts.write(lr.sounds_offset);
		opts.write(lr.skydome_unit);

		// Write level sounds
		for (u32 i = 0; i < array::size(sounds); ++i) {
			opts.write(sounds[i].name);
			opts.write(sounds[i].position);
			opts.write(sounds[i].volume);
			opts.write(sounds[i].range);
			opts.write(sounds[i].loop);
			opts.write(sounds[i].group);
			opts.write(sounds[i]._pad);
		}

		// Write unit names
		for (u32 i = 0; i < array::size(uc._unit_names); ++i)
			opts.write(uc._unit_names[i]._id);

		// Write units
		opts.align(16);
		opts.write(units_blob);
		return 0;
	}

} // namespace level_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
