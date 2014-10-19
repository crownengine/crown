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

#include "types.h"
#include "string_utils.h"
#include "string_utils.h"
#include "compile_options.h"

namespace crown
{

/// Hashed values for supported resource types
#define CONFIG_EXTENSION			"config"
#define FONT_EXTENSION				"font"
#define LEVEL_EXTENSION				"level"
#define LUA_EXTENSION				"lua"
#define MATERIAL_EXTENSION			"material"
#define MESH_EXTENSION				"mesh"
#define PACKAGE_EXTENSION			"package"
#define PHYSICS_CONFIG_EXTENSION	"physics_config"
#define PHYSICS_EXTENSION			"physics"
#define SHADER_EXTENSION			"shader"
#define SOUND_EXTENSION				"sound"
#define SPRITE_ANIMATION_EXTENSION	"sprite_animation"
#define SPRITE_EXTENSION			"sprite"
#define TEXTURE_EXTENSION			"texture"
#define UNIT_EXTENSION				"unit"

#define CONFIG_TYPE					uint64_t(0x82645835e6b73232)
#define FONT_TYPE					uint64_t(0x9efe0a916aae7880)
#define LEVEL_TYPE					uint64_t(0x2a690fd348fe9ac5)
#define LUA_TYPE					uint64_t(0xa14e8dfa2cd117e2)
#define MATERIAL_TYPE				uint64_t(0xeac0b497876adedf)
#define MESH_TYPE					uint64_t(0x48ff313713a997a1)
#define PACKAGE_TYPE				uint64_t(0xad9c6d9ed1e5e77a)
#define PHYSICS_CONFIG_TYPE			uint64_t(0x72e3cc03787a11a1)
#define PHYSICS_TYPE				uint64_t(0x5f7203c8f280dab8)
#define SHADER_TYPE					uint64_t(0xcce8d5b5f5ae333f)
#define SOUND_TYPE					uint64_t(0x90641b51c98b7aac)
#define SPRITE_ANIMATION_TYPE		uint64_t(0x487e78e3f87f238d)
#define SPRITE_TYPE					uint64_t(0x8d5871f9ebdb651c)
#define TEXTURE_TYPE				uint64_t(0xcd4238c6a0c69e32)
#define UNIT_TYPE					uint64_t(0xe0a48d0be9a7453f)

/*#define CONFIG_VERSION				uint32_t(1)
#define FONT_VERSION				uint32_t(1)
#define LEVEL_VERSION				uint32_t(1)
#define LUA_VERSION					uint32_t(1)
#define MATERIAL_VERSION			uint32_t(1)
#define MESH_VERSION				uint32_t(1)
#define PACKAGE_VERSION				uint32_t(1)
#define PHYSICS_CONFIG_VERSION		uint32_t(1)
#define PHYSICS_VERSION				uint32_t(1)
#define SHADER_VERSION				uint32_t(1)
#define SOUND_VERSION				uint32_t(1)
#define SPRITE_ANIMATION_VERSION	uint32_t(1)
#define SPRITE_VERSION				uint32_t(1)
#define TEXTURE_VERSION				uint32_t(1)
#define UNIT_VERSION				uint32_t(1)*/

struct ResourceId
{
	ResourceId() : type(0), name(0) {}
	ResourceId(uint64_t type, uint64_t name) : type(type), name(name) {}
	ResourceId(const char* type, const char* name);

	bool operator==(const ResourceId& a) const { return type == a.type && name == a.name; }
	bool operator<(const ResourceId& a) const { return type < a.type || (type == a.type && name < a.name); }

	uint64_t type;
	uint64_t name;
};

class Allocator;
class Bundle;
class ResourceManager;
class Filesystem;
class File;

} // namespace crown
