/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include <bgfx/bgfx.h>

namespace crown
{

struct MeshResource
{
	bgfx::VertexDecl decl;
	const bgfx::Memory* vbmem;
	const bgfx::Memory* ibmem;
	bgfx::VertexBufferHandle vb;
	bgfx::IndexBufferHandle ib;
};

namespace mesh_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& a, void* res);
}
} // namespace crown
