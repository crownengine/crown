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

#include "level_resource.h"
#include "array.h"
#include "memory.h"
#include "json_parser.h"
#include "filesystem.h"

namespace crown
{
namespace level_resource
{
	//-----------------------------------------------------------------------------
	void parse_units(JSONElement root, Array<LevelUnit>& units)
	{
		JSONElement units_arr = root.key("units");
		const uint32_t size = units_arr.size();

		for (uint32_t i = 0; i < size; i++)
		{
			JSONElement e = units_arr[i];

			LevelUnit lu;
			lu.name = e.key("name").to_resource_id("unit");
			lu.position = e.key("position").to_vector3();
			lu.rotation = e.key("rotation").to_quaternion();

			array::push_back(units, lu);
		}
	}

	//-----------------------------------------------------------------------------
	void parse_sounds(JSONElement root, Array<LevelSound>& sounds)
	{
		JSONElement sounds_arr = root.key("sounds");
		const uint32_t size = sounds_arr.size();

		for (uint32_t i = 0; i < size; i++)
		{
			JSONElement e = sounds_arr[i];

			LevelSound ls;
			ls.name = e.key("name").to_resource_id("sound");
			ls.position = e.key("position").to_vector3();
			ls.volume = e.key("volume").to_float();
			ls.range = e.key("range").to_float();
			ls.loop = e.key("loop").to_bool();

			array::push_back(sounds, ls);
		}
	}

	//-----------------------------------------------------------------------------
	void compile(Filesystem& fs, const char* resource_path, File* out_file)
	{
		File* file = fs.open(resource_path, FOM_READ);
		JSONParser json(*file);
		fs.close(file);

		JSONElement root = json.root();

		Array<LevelUnit> units(default_allocator());
		Array<LevelSound> sounds(default_allocator());

		parse_units(root, units);
		parse_sounds(root, sounds);

		LevelHeader lh;
		lh.num_units = array::size(units);
		lh.num_sounds = array::size(sounds);

		uint32_t offt = sizeof(LevelHeader);
		lh.units_offset = offt; offt += sizeof(LevelUnit) * lh.num_units;
		lh.sounds_offset = offt;

		out_file->write((char*) &lh, sizeof(LevelHeader));

		if (lh.num_units)
		{
			out_file->write((char*) array::begin(units), sizeof(LevelUnit) * lh.num_units);
		}
		if (lh.num_sounds)
		{
			out_file->write((char*) array::begin(sounds), sizeof(LevelSound) * lh.num_sounds);
		}
	}

	//-----------------------------------------------------------------------------
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	//-----------------------------------------------------------------------------
	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}
} // namespace level_resource
} // namespace crown
