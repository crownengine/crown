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

#include "Types.h"
#include "Hash.h"
#include "StringUtils.h"

namespace crown
{

/// Hashed values for supported resource types
#define TEXTURE_EXTENSION			"texture"
#define MESH_EXTENSION				"mesh"
#define LUA_EXTENSION				"lua"
#define TEXT_EXTENSION				"text"
#define MATERIAL_EXTENSION			"material"
#define SOUND_EXTENSION				"sound"
#define SPRITE_EXTENSION			"sprite"
#define CONFIG_EXTENSION			"config"
#define PACKAGE_EXTENSION			"package"
#define UNIT_EXTENSION				"unit"
#define PHYSICS_EXTENSION			"physics"
#define GUI_EXTENSION				"gui"
#define PHYSICS_CONFIG_EXTENSION	"physics_config"
#define FONT_EXTENSION				"font"

#define TEXTURE_TYPE				0x0DEED4F7
#define MESH_TYPE					0x742FBC9A
#define LUA_TYPE					0xD96E7C37
#define TEXT_TYPE					0x045CC650
#define MATERIAL_TYPE				0x46807A92
#define SOUND_TYPE					0xD196AB6E
#define SPRITE_TYPE					0x5DD272E5
#define CONFIG_TYPE					0x17DEA5E1
#define PACKAGE_TYPE				0xC0A2212C
#define UNIT_TYPE					0x516224CF
#define PHYSICS_TYPE				0xFA32C012
#define GUI_TYPE					0x2C56149A
#define PHYSICS_CONFIG_TYPE			0x514F14A1
#define FONT_TYPE					0x536DC7D4

/// ResourceId uniquely identifies a resource by its name and type.
/// In order to speed up the lookup by the manager, it also keeps
/// the index to the resource list where it is stored.
struct ResourceId
{
	bool operator==(const ResourceId& b) const { return id == b.id; }

	uint64_t id;
};

class Allocator;
class Bundle;

typedef void*	(*ResourceLoadCallback)(Allocator& a, Bundle& b, ResourceId id);
typedef void	(*ResourceUnloadCallback)(Allocator& a, void* resource);
typedef void	(*ResourceOnlineCallback)(void* resource);
typedef void	(*ResourceOfflineCallback)(void* resource);

struct ResourceCallback
{
	uint32_t					type;
	ResourceLoadCallback		on_load;
	ResourceUnloadCallback		on_unload;
	ResourceOnlineCallback		on_online;
	ResourceOfflineCallback		on_offline;
};

} // namespace crown

