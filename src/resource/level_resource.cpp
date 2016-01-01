/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "level_resource.h"
#include "array.h"
#include "memory.h"
#include "json_parser.h"
#include "filesystem.h"
#include "compile_options.h"

namespace crown
{
namespace level_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		Array<LevelUnit> units(default_allocator());
		Array<LevelSound> sounds(default_allocator());

		{
			JSONElement sounds_arr = root.key("sounds");
			const uint32_t size = sounds_arr.size();
			for (uint32_t i = 0; i < size; i++)
			{
				JSONElement e = sounds_arr[i];
				LevelSound ls;
				ls.name = e.key("name").to_resource_id();
				ls.position = e.key("position").to_vector3();
				ls.volume = e.key("volume").to_float();
				ls.range = e.key("range").to_float();
				ls.loop = e.key("loop").to_bool();
				array::push_back(sounds, ls);
			}
		}

		{
			JSONElement units_arr = root.key("units");
			const uint32_t size = units_arr.size();
			for (uint32_t i = 0; i < size; i++)
			{
				JSONElement e = units_arr[i];
				LevelUnit lu;
				lu.name = e.key("name").to_resource_id();
				lu.position = e.key("position").to_vector3();
				lu.rotation = e.key("rotation").to_quaternion();
				array::push_back(units, lu);
			}
		}

		LevelResource lr;
		lr.version = LEVEL_VERSION;
		lr.num_units = array::size(units);
		lr.num_sounds = array::size(sounds);

		uint32_t offt = sizeof(LevelResource);
		lr.units_offset = offt; offt += sizeof(LevelUnit) * lr.num_units;
		lr.sounds_offset = offt;

		opts.write(lr.version);
		opts.write(lr.num_units);
		opts.write(lr.units_offset);
		opts.write(lr.num_sounds);
		opts.write(lr.sounds_offset);

		for (uint32_t i = 0; i < array::size(units); ++i)
		{
			opts.write(units[i].name);
			opts.write(units[i].position);
			opts.write(units[i].rotation);
			opts.write(units[i]._pad);
		}

		for (uint32_t i = 0; i < array::size(sounds); ++i)
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
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == LEVEL_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t num_units(const LevelResource* lr)
	{
		return lr->num_units;
	}

	const LevelUnit* get_unit(const LevelResource* lr, uint32_t i)
	{
		CE_ASSERT(i < num_units(lr), "Index out of bounds");
		const LevelUnit* begin = (LevelUnit*)((char*)lr + lr->units_offset);
		return &begin[i];
	}

	uint32_t num_sounds(const LevelResource* lr)
	{
		return lr->num_sounds;
	}

	const LevelSound* get_sound(const LevelResource* lr, uint32_t i)
	{
		CE_ASSERT(i < num_sounds(lr), "Index out of bounds");
		const LevelSound* begin = (LevelSound*)((char*)lr + lr->sounds_offset);
		return &begin[i];
	}
} // namespace level_resource
} // namespace crown
