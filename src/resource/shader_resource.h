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

struct ShaderBackend
{
	enum Enum
	{
		ESSL,
		GLSL,
		HLSL,
		SPIRV,

		COUNT
	};
};

struct ShaderResource
{
	enum { MAX_SAMPLERS = 16 };

	explicit ShaderResource(Allocator &a)
		: _data(a)
	{
	}

	~ShaderResource();

	struct Sampler
	{
		u32 name;
		u32 state;
		u32 stage;
	};

	struct Code
	{
		u32 backend;
		u32 vs_size;
		u8 *vs_data;
		u32 fs_size;
		u8 *fs_data;
	};

	struct Data
	{
		StringId32 name;
		u64 state;
		u32 stencil_front;
		u32 stencil_back;
		u32 num_samplers;
		Sampler samplers[MAX_SAMPLERS];
		u32 num_codes;
		Code codes[ShaderBackend::COUNT];
	};

	Array<Data> _data;
};

struct ShaderData
{
	u64 state;
	u32 stencil_front;
	u32 stencil_back;
	u32 num_samplers;
	ShaderResource::Sampler samplers[ShaderResource::MAX_SAMPLERS];
	bgfx::ProgramHandle program;
#if CROWN_CAN_RELOAD
	const ShaderResource *resource;
#endif
};

#if CROWN_CAN_COMPILE
namespace shader_compiler
{
	/// Clears cached shader metadata used by material compilation.
	void clear_metadata_cache();

	/// Compiles a @a shader variant and writes it to @a fb. The shader must be defined inside @a
	/// shader_library. If @a shader_library is empty it tries to find a suitable one in the source
	/// directories and returns it via @a shader_library itself.
	s32 compile_variant(FileBuffer &fb
		, Vector<UniformMetadata> *uniform_meta
		, DynamicString &shader_library
		, StringId32 &shader_id
		, StringView &shader
		, Vector<StringView> &defines
		, CompileOptions &opts
		, bool metadata_only = false
		, Vector<ShaderResource::Sampler> *sampler_meta = NULL
		);

} // namespace shader_compiler
#endif // if CROWN_CAN_COMPILE

} // namespace crown
