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
const char* const TEXTURE_EXTENSION			= "texture";
const char* const MESH_EXTENSION			= "mesh";
const char* const LUA_EXTENSION				= "lua";
const char* const TEXT_EXTENSION			= "text";
const char* const MATERIAL_EXTENSION		= "material";
const char* const SOUND_EXTENSION			= "sound";
const char* const SPRITE_EXTENSION			= "sprite";
const char* const CONFIG_EXTENSION			= "config";
const char* const PACKAGE_EXTENSION			= "package";
const char* const UNIT_EXTENSION			= "unit";
const char* const PHYSICS_EXTENSION			= "physics";
const char* const GUI_EXTENSION				= "gui";

const uint32_t TEXTURE_TYPE					= 0x0DEED4F7;
const uint32_t MESH_TYPE					= 0x742FBC9A;
const uint32_t LUA_TYPE						= 0xD96E7C37;
const uint32_t TEXT_TYPE					= 0x045CC650;
const uint32_t MATERIAL_TYPE				= 0x46807A92;
const uint32_t SOUND_TYPE					= 0xD196AB6E;
const uint32_t SPRITE_TYPE					= 0x5DD272E5;
const uint32_t CONFIG_TYPE					= 0x17DEA5E1;
const uint32_t PACKAGE_TYPE					= 0xC0A2212C;
const uint32_t UNIT_TYPE					= 0x516224CF;
const uint32_t PHYSICS_TYPE					= 0xFA32C012;
const uint32_t GUI_TYPE						= 0x2C56149A;

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

