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

#pragma once

#include "resource.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "math_types.h"

namespace crown
{

struct LevelResource
{
	uint32_t version;
	uint32_t num_units;
	uint32_t units_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
};

struct LevelUnit
{
	StringId64 name;
	Vector3 position;
	Quaternion rotation;
	uint32_t _pad;
};

struct LevelSound
{
	StringId64 name;
	Vector3 position;
	float volume;
	float range;
	bool loop;
	char _pad[3];
};

namespace level_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	uint32_t num_units(const LevelResource* lr);
	const LevelUnit* get_unit(const LevelResource* lr, uint32_t i);
	uint32_t num_sounds(const LevelResource* lr);
	const LevelSound* get_sound(const LevelResource* lr, uint32_t i);
} // namespace level_resource
} // namespace crown
