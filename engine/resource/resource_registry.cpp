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

#include "resource_registry.h"
#include "lua_resource.h"
#include "texture_resource.h"
#include "mesh_resource.h"
#include "sound_resource.h"
#include "sprite_resource.h"
#include "package_resource.h"
#include "unit_resource.h"
#include "physics_resource.h"
#include "material_resource.h"
#include "font_resource.h"
#include "level_resource.h"
#include "shader.h"

namespace crown
{

namespace pcr = physics_config_resource;
namespace phr = physics_resource;
namespace pkr = package_resource;
namespace sdr = sound_resource;
namespace mhr = mesh_resource;
namespace utr = unit_resource;
namespace txr = texture_resource;
namespace mtr = material_resource;
namespace lur = lua_resource;
namespace ftr = font_resource;
namespace lvr = level_resource;
namespace spr = sprite_resource;
namespace shr = shader_resource;
namespace sar = sprite_animation_resource;

typedef void  (*ResourceCompileCallback)(const char* path, CompileOptions& opts);
typedef void* (*ResourceLoadCallback)(Allocator& a, Bundle& b, ResourceId id);
typedef void  (*ResourceOnlineCallback)(StringId64 id, ResourceManager& rm);
typedef void  (*ResourceOfflineCallback)(StringId64 id, ResourceManager& rm);
typedef void  (*ResourceUnloadCallback)(Allocator& a, void* resource);

struct ResourceCallback
{
	uint64_t type;
	ResourceCompileCallback on_compile;
	ResourceLoadCallback on_load;
	ResourceUnloadCallback on_unload;
	ResourceOnlineCallback on_online;
	ResourceOfflineCallback on_offline;
};

static const ResourceCallback RESOURCE_CALLBACK_REGISTRY[] =
{
	{ LUA_TYPE,              lur::compile, lur::load, lur::unload, lur::online, lur::offline },
	{ TEXTURE_TYPE,          txr::compile, txr::load, txr::unload, txr::online, txr::offline },
	{ MESH_TYPE,             mhr::compile, mhr::load, mhr::unload, mhr::online, mhr::offline },
	{ SOUND_TYPE,            sdr::compile, sdr::load, sdr::unload, sdr::online, sdr::offline },
	{ UNIT_TYPE,             utr::compile, utr::load, utr::unload, utr::online, utr::offline },
	{ SPRITE_TYPE,           spr::compile, spr::load, spr::unload, spr::online, spr::offline },
	{ PACKAGE_TYPE,          pkr::compile, pkr::load, pkr::unload, pkr::online, pkr::offline },
	{ PHYSICS_TYPE,          phr::compile, phr::load, phr::unload, phr::online, phr::offline },
	{ MATERIAL_TYPE,         mtr::compile, mtr::load, mtr::unload, mtr::online, mtr::offline },
	{ PHYSICS_CONFIG_TYPE,   pcr::compile, pcr::load, pcr::unload, pcr::online, pcr::offline },
	{ FONT_TYPE,             ftr::compile, ftr::load, ftr::unload, ftr::online, ftr::offline },
	{ LEVEL_TYPE,            lvr::compile, lvr::load, lvr::unload, lvr::online, lvr::offline },
	{ SHADER_TYPE,           shr::compile, shr::load, shr::unload, shr::online, shr::offline },
	{ SPRITE_ANIMATION_TYPE, sar::compile, sar::load, sar::unload, sar::online, sar::offline },
	{ 0,                     NULL,         NULL,      NULL,        NULL,        NULL         }
};

//-----------------------------------------------------------------------------
static const ResourceCallback* find_callback(uint64_t type)
{
	const ResourceCallback* c = RESOURCE_CALLBACK_REGISTRY;

	while (c->type != 0 && c->type != type)
	{
		c++;
	}

	CE_ASSERT_NOT_NULL(c);
	return c;
}

void resource_on_compile(uint64_t type, const char* path, CompileOptions& opts)
{
	return find_callback(type)->on_compile(path, opts);
}

//-----------------------------------------------------------------------------
void* resource_on_load(uint64_t type, Allocator& allocator, Bundle& bundle, ResourceId id)
{
	return find_callback(type)->on_load(allocator, bundle, id);
}

//-----------------------------------------------------------------------------
void resource_on_unload(uint64_t type, Allocator& allocator, void* resource)
{
	return find_callback(type)->on_unload(allocator, resource);
}

//-----------------------------------------------------------------------------
void resource_on_online(uint64_t type, StringId64 id, ResourceManager& rm)
{
	return find_callback(type)->on_online(id, rm);
}

//-----------------------------------------------------------------------------
void resource_on_offline(uint64_t type, StringId64 id, ResourceManager& rm)
{
	return find_callback(type)->on_offline(id, rm);
}

} // namespace crown
