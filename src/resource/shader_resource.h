/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/strings/types.h"
#include "core/types.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,
		MATRIX4X4,

		COUNT
	};
};

struct UniformMetadata
{
	UniformType::Enum type;
	char name[128];
	Vector4 val;
};

struct ShaderResource
{
	explicit ShaderResource(Allocator &a)
		: _data(a)
	{
	}

	struct Sampler
	{
		u32 name;
		u32 state;
	};

	struct Data
	{
		StringId32 name;
		u64 state;
		u32 stencil_front;
		u32 stencil_back;
		Sampler samplers[16];
		const bgfx::Memory *vsmem;
		const bgfx::Memory *fsmem;
	};

	Array<Data> _data;
};

struct ShaderData
{
	u64 state;
	u32 stencil_front;
	u32 stencil_back;
	ShaderResource::Sampler samplers[4];
	bgfx::ProgramHandle program;
#if CROWN_CAN_RELOAD
	const ShaderResource *resource;
#endif
};

#if CROWN_CAN_COMPILE
namespace shader_compiler
{
	/// Compiles a @a shader variant and writes it to @a fb. The shader must be defined inside @a
	/// shader_library. If @a shader_library is empty it tries to find a suitable one in the source
	/// directories and returns it via @a shader_library itself.
	s32 compile_variant(FileBuffer &fb
		, Vector<UniformMetadata> *uniform_meta
		, DynamicString &shader_library
		, StringView &shader
		, Vector<StringView> &defines
		, CompileOptions &opts
		, bool metadata_only = false
		);

} // namespace shader_compiler
#endif

} // namespace crown
