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

template <typename STREAM, typename T> inline STREAM& operator&(STREAM& stream, T& t)
{
	return t.serialize(stream);
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, bool& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, bool& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, float& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, float& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, char& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, char& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, int8_t& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, int8_t& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, uint8_t& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, uint8_t& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, uint32_t& v)
{
	bw.write(v);
	return bw;
}

template <> inline BinaryReader& operator&(BinaryReader& br, uint32_t& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryWriter& operator&(BinaryWriter& br, uint64_t& v)
{
	br.write(v);
	return br;
}

template <> inline BinaryReader& operator&(BinaryReader& br, uint64_t& v)
{
	br.read(v);
	return br;
}

template <> inline BinaryReader& operator&(BinaryReader& br, ResourceId& id)
{
	return br & id.type & id.name;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, Vector3& v)
{
	return bw & v.x & v.y & v.z;
}

template <> inline BinaryReader& operator&(BinaryReader& br, Vector3& v)
{
	return br & v.x & v.y & v.z;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, Quaternion& q)
{
	return bw & q.x & q.y & q.z & q.w;
}

template <> inline BinaryReader& operator&(BinaryReader& br, Quaternion& q)
{
	return br & q.x & q.y & q.z & q.w;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, LevelResource& data)
{
	return bw
		& data.version
		& data.num_units
		& data.units_offset
		& data.num_sounds
		& data.sounds_offset;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, LevelUnit& data)
{
	return bw
		& data.name
		& data.position
		& data.rotation
		& data._pad;
}

template <> inline BinaryWriter& operator&(BinaryWriter& bw, LevelSound& data)
{
	return bw
		& data.name
		& data.position
		& data.volume
		& data.range
		& data.loop
		& data._pad[0]
		& data._pad[1]
		& data._pad[2];
}

template <typename T> inline BinaryWriter& operator&(BinaryWriter& bw, Array<T> arr)
{
	for (uint32_t i = 0; i < array::size(arr); i++)
		bw & arr[i];
	return bw;
}

namespace level_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
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
				ls.name = e.key("name").to_resource_id("sound").name;
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
				lu.name = e.key("name").to_resource_id("unit").name;
				lu.position = e.key("position").to_vector3();
				lu.rotation = e.key("rotation").to_quaternion();
				array::push_back(units, lu);
			}
		}

		LevelResource lr;
		lr.version = VERSION;
		lr.num_units = array::size(units);
		lr.num_sounds = array::size(sounds);

		uint32_t offt = sizeof(LevelResource);
		lr.units_offset = offt; offt += sizeof(LevelUnit) * lr.num_units;
		lr.sounds_offset = offt;

		opts._bw & lr
			& units
			& sounds;
	}

	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
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
