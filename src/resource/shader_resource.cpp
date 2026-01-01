/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/json/json.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/option.inl"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "device/device.h"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/expression_language.inl"
#include "resource/resource_manager.h"
#include "resource/shader_resource.h"
#include "world/shader_manager.h"
#include <algorithm> // std::sort

LOG_SYSTEM(SHADER_RESOURCE, "shader_resource")

namespace crown
{
namespace shader_resource_internal
{
	void *load(File &file, Allocator &a)
	{
		return device()->_shader_manager->load(file, a);
	}

	void online(StringId64 id, ResourceManager &rm)
	{
		device()->_shader_manager->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager &rm)
	{
		device()->_shader_manager->offline(id, rm);
	}

	void unload(Allocator &a, void *res)
	{
		device()->_shader_manager->unload(a, res);
	}

} // namespace shader_resource_internal

#if CROWN_CAN_COMPILE
namespace shader_resource_internal
{
	static const char *shaderc_platform[] =
	{
		"android", // Platform::ANDROID
		"android", // Platform::ANDROID_ARM64
		"asm.js",  // Platform::HTML5
		"linux",   // Platform::LINUX
		"windows"  // Platform::WINDOWS
	};
	CE_STATIC_ASSERT(countof(shaderc_platform) == Platform::COUNT);

	static const char *shaderc_paths[] =
	{
		EXE_PATH("shaderc"),
#if CROWN_DEBUG
		EXE_PATH("shaderc-debug")
#elif CROWN_DEVELOPMENT
		EXE_PATH("shaderc-development")
#else
		EXE_PATH("shaderc-release")
#endif
	};

	struct DepthFunction
	{
		enum Enum
		{
			LESS,
			LEQUAL,
			EQUAL,
			GEQUAL,
			GREATER,
			NOTEQUAL,
			NEVER,
			ALWAYS,

			COUNT
		};
	};

	struct BlendFunction
	{
		enum Enum
		{
			ZERO,
			ONE,
			SRC_COLOR,
			INV_SRC_COLOR,
			SRC_ALPHA,
			INV_SRC_ALPHA,
			DST_ALPHA,
			INV_DST_ALPHA,
			DST_COLOR,
			INV_DST_COLOR,
			SRC_ALPHA_SAT,
			FACTOR,
			INV_FACTOR,

			COUNT
		};
	};

	struct BlendEquation
	{
		enum Enum
		{
			ADD,
			SUB,
			REVSUB,
			MIN,
			MAX,

			COUNT
		};
	};

	struct CullMode
	{
		enum Enum
		{
			CW,
			CCW,
			NONE,

			COUNT
		};
	};

	struct PrimitiveType
	{
		enum Enum
		{
			PT_TRIANGLES,
			PT_TRISTRIP,
			PT_LINES,
			PT_LINESTRIP,
			PT_POINTS,

			COUNT
		};
	};

	struct SamplerFilter
	{
		enum Enum
		{
			POINT,
			ANISOTROPIC,

			COUNT
		};
	};

	struct SamplerWrap
	{
		enum Enum
		{
			MIRROR,
			CLAMP,
			BORDER,

			COUNT
		};
	};

	struct DepthTestInfo
	{
		const char *name;
		DepthFunction::Enum value;
	};

	static const DepthTestInfo _depth_test_map[] =
	{
		{ "less",     DepthFunction::LESS     },
		{ "lequal",   DepthFunction::LEQUAL   },
		{ "equal",    DepthFunction::EQUAL    },
		{ "gequal",   DepthFunction::GEQUAL   },
		{ "greater",  DepthFunction::GREATER  },
		{ "notequal", DepthFunction::NOTEQUAL },
		{ "never",    DepthFunction::NEVER    },
		{ "always",   DepthFunction::ALWAYS   }
	};
	CE_STATIC_ASSERT(countof(_depth_test_map) == DepthFunction::COUNT);

	struct BlendFunctionInfo
	{
		const char *name;
		BlendFunction::Enum value;
	};

	static const BlendFunctionInfo _blend_func_map[] =
	{
		{ "zero",          BlendFunction::ZERO          },
		{ "one",           BlendFunction::ONE           },
		{ "src_color",     BlendFunction::SRC_COLOR     },
		{ "inv_src_color", BlendFunction::INV_SRC_COLOR },
		{ "src_alpha",     BlendFunction::SRC_ALPHA     },
		{ "inv_src_alpha", BlendFunction::INV_SRC_ALPHA },
		{ "dst_alpha",     BlendFunction::DST_ALPHA     },
		{ "inv_dst_alpha", BlendFunction::INV_DST_ALPHA },
		{ "dst_color",     BlendFunction::DST_COLOR     },
		{ "inv_dst_color", BlendFunction::INV_DST_COLOR },
		{ "src_alpha_sat", BlendFunction::SRC_ALPHA_SAT },
		{ "factor",        BlendFunction::FACTOR        },
		{ "inv_factor",    BlendFunction::INV_FACTOR    }
	};
	CE_STATIC_ASSERT(countof(_blend_func_map) == BlendFunction::COUNT);

	struct BlendEquationInfo
	{
		const char *name;
		BlendEquation::Enum value;
	};

	static const BlendEquationInfo _blend_equation_map[] =
	{
		{ "add",    BlendEquation::ADD    },
		{ "sub",    BlendEquation::SUB    },
		{ "revsub", BlendEquation::REVSUB },
		{ "min",    BlendEquation::MIN    },
		{ "max",    BlendEquation::MAX    }
	};
	CE_STATIC_ASSERT(countof(_blend_equation_map) == BlendEquation::COUNT);

	struct CullModeInfo
	{
		const char *name;
		CullMode::Enum value;
	};

	static const CullModeInfo _cull_mode_map[] =
	{
		{ "cw",   CullMode::CW   },
		{ "ccw",  CullMode::CCW  },
		{ "none", CullMode::NONE }
	};
	CE_STATIC_ASSERT(countof(_cull_mode_map) == CullMode::COUNT);

	struct PrimitiveTypeInfo
	{
		const char *name;
		PrimitiveType::Enum value;
	};

	static const PrimitiveTypeInfo _primitive_type_map[] =
	{
		{ "pt_triangles", PrimitiveType::PT_TRIANGLES },
		{ "pt_tristrip",  PrimitiveType::PT_TRISTRIP  },
		{ "pt_lines",     PrimitiveType::PT_LINES     },
		{ "pt_linestrip", PrimitiveType::PT_LINESTRIP },
		{ "pt_points",    PrimitiveType::PT_POINTS    }
	};
	CE_STATIC_ASSERT(countof(_primitive_type_map) == PrimitiveType::COUNT);

	struct SamplerFilterInfo
	{
		const char *name;
		SamplerFilter::Enum value;
	};

	static const SamplerFilterInfo _sampler_filter_map[] =
	{
		{ "point",       SamplerFilter::POINT       },
		{ "anisotropic", SamplerFilter::ANISOTROPIC }
	};
	CE_STATIC_ASSERT(countof(_sampler_filter_map) == SamplerFilter::COUNT);

	struct SamplerWrapInfo
	{
		const char *name;
		SamplerWrap::Enum value;
	};

	static const SamplerWrapInfo _sampler_wrap_map[] =
	{
		{ "mirror", SamplerWrap::MIRROR },
		{ "clamp",  SamplerWrap::CLAMP  },
		{ "border", SamplerWrap::BORDER }
	};
	CE_STATIC_ASSERT(countof(_sampler_wrap_map) == SamplerWrap::COUNT);

	static const u64 _bgfx_depth_func_map[] =
	{
		BGFX_STATE_DEPTH_TEST_LESS,     // DepthFunction::LESS
		BGFX_STATE_DEPTH_TEST_LEQUAL,   // DepthFunction::LEQUAL
		BGFX_STATE_DEPTH_TEST_EQUAL,    // DepthFunction::EQUAL
		BGFX_STATE_DEPTH_TEST_GEQUAL,   // DepthFunction::GEQUAL
		BGFX_STATE_DEPTH_TEST_GREATER,  // DepthFunction::GREATER
		BGFX_STATE_DEPTH_TEST_NOTEQUAL, // DepthFunction::NOTEQUAL
		BGFX_STATE_DEPTH_TEST_NEVER,    // DepthFunction::NEVER
		BGFX_STATE_DEPTH_TEST_ALWAYS    // DepthFunction::ALWAYS
	};
	CE_STATIC_ASSERT(countof(_bgfx_depth_func_map) == DepthFunction::COUNT);

	static const u64 _bgfx_blend_func_map[] =
	{
		BGFX_STATE_BLEND_ZERO,          // BlendFunction::ZERO
		BGFX_STATE_BLEND_ONE,           // BlendFunction::ONE
		BGFX_STATE_BLEND_SRC_COLOR,     // BlendFunction::SRC_COLOR
		BGFX_STATE_BLEND_INV_SRC_COLOR, // BlendFunction::INV_SRC_COLOR
		BGFX_STATE_BLEND_SRC_ALPHA,     // BlendFunction::SRC_ALPHA
		BGFX_STATE_BLEND_INV_SRC_ALPHA, // BlendFunction::INV_SRC_ALPHA
		BGFX_STATE_BLEND_DST_ALPHA,     // BlendFunction::DST_ALPHA
		BGFX_STATE_BLEND_INV_DST_ALPHA, // BlendFunction::INV_DST_ALPHA
		BGFX_STATE_BLEND_DST_COLOR,     // BlendFunction::DST_COLOR
		BGFX_STATE_BLEND_INV_DST_COLOR, // BlendFunction::INV_DST_COLOR
		BGFX_STATE_BLEND_SRC_ALPHA_SAT, // BlendFunction::SRC_ALPHA_SAT
		BGFX_STATE_BLEND_FACTOR,        // BlendFunction::FACTOR
		BGFX_STATE_BLEND_INV_FACTOR     // BlendFunction::INV_FACTOR
	};
	CE_STATIC_ASSERT(countof(_bgfx_blend_func_map) == BlendFunction::COUNT);

	static const u64 _bgfx_blend_equation_map[] =
	{
		BGFX_STATE_BLEND_EQUATION_ADD,    // BlendEquation::ADD
		BGFX_STATE_BLEND_EQUATION_SUB,    // BlendEquation::SUB
		BGFX_STATE_BLEND_EQUATION_REVSUB, // BlendEquation::REVSUB
		BGFX_STATE_BLEND_EQUATION_MIN,    // BlendEquation::MIN
		BGFX_STATE_BLEND_EQUATION_MAX     // BlendEquation::MAX
	};
	CE_STATIC_ASSERT(countof(_bgfx_blend_equation_map) == BlendEquation::COUNT);

	static const u64 _bgfx_cull_mode_map[] =
	{
		BGFX_STATE_CULL_CW,  // CullMode::CW
		BGFX_STATE_CULL_CCW, // CullMode::CCW
		0                    // CullMode::NONE
	};
	CE_STATIC_ASSERT(countof(_bgfx_cull_mode_map) == CullMode::COUNT);

	static const u64 _bgfx_primitive_type_map[] =
	{
		0,                       // PrimitiveType::PT_TRIANGLES
		BGFX_STATE_PT_TRISTRIP,  // PrimitiveType::PT_TRISTRIP
		BGFX_STATE_PT_LINES,     // PrimitiveType::PT_LINES
		BGFX_STATE_PT_LINESTRIP, // PrimitiveType::PT_LINESTRIP
		BGFX_STATE_PT_POINTS     // PrimitiveType::PT_POINTS
	};
	CE_STATIC_ASSERT(countof(_bgfx_primitive_type_map) == PrimitiveType::COUNT);

	static const u32 _bgfx_sampler_filter_min_map[] =
	{
		BGFX_SAMPLER_MIN_POINT,      // SamplerFilter::POINT
		BGFX_SAMPLER_MIN_ANISOTROPIC // SamplerFilter::ANISOTROPIC
	};
	CE_STATIC_ASSERT(countof(_bgfx_sampler_filter_min_map) == SamplerFilter::COUNT);

	static const u32 _bgfx_sampler_filter_mag_map[] =
	{
		BGFX_SAMPLER_MAG_POINT,      // SamplerFilter::POINT
		BGFX_SAMPLER_MAG_ANISOTROPIC // SamplerFilter::ANISOTROPIC
	};
	CE_STATIC_ASSERT(countof(_bgfx_sampler_filter_mag_map) == SamplerFilter::COUNT);

	static const u32 _bgfx_sampler_wrap_u_map[] =
	{
		BGFX_SAMPLER_U_MIRROR, // SamplerWrap::MIRROR
		BGFX_SAMPLER_U_CLAMP,  // SamplerWrap::CLAMP
		BGFX_SAMPLER_U_BORDER  // SamplerWrap::BORDER
	};
	CE_STATIC_ASSERT(countof(_bgfx_sampler_wrap_u_map) == SamplerWrap::COUNT);

	static const u32 _bgfx_sampler_wrap_v_map[] =
	{
		BGFX_SAMPLER_V_MIRROR, // SamplerWrap::MIRROR
		BGFX_SAMPLER_V_CLAMP,  // SamplerWrap::CLAMP
		BGFX_SAMPLER_V_BORDER  // SamplerWrap::BORDER
	};
	CE_STATIC_ASSERT(countof(_bgfx_sampler_wrap_v_map) == SamplerWrap::COUNT);

	static const u32 _bgfx_sampler_wrap_w_map[] =
	{
		BGFX_SAMPLER_W_MIRROR, // SamplerWrap::MIRROR
		BGFX_SAMPLER_W_CLAMP,  // SamplerWrap::CLAMP
		BGFX_SAMPLER_W_BORDER  // SamplerWrap::BORDER
	};
	CE_STATIC_ASSERT(countof(_bgfx_sampler_wrap_w_map) == SamplerWrap::COUNT);

	static DepthFunction::Enum name_to_depth_func(const char *name)
	{
		for (u32 i = 0; i < countof(_depth_test_map); ++i) {
			if (strcmp(name, _depth_test_map[i].name) == 0)
				return _depth_test_map[i].value;
		}

		return DepthFunction::COUNT;
	}

	static BlendFunction::Enum name_to_blend_function(const char *name)
	{
		for (u32 i = 0; i < countof(_blend_func_map); ++i) {
			if (strcmp(name, _blend_func_map[i].name) == 0)
				return _blend_func_map[i].value;
		}

		return BlendFunction::COUNT;
	}

	static BlendEquation::Enum name_to_blend_equation(const char *name)
	{
		for (u32 i = 0; i < countof(_blend_equation_map); ++i) {
			if (strcmp(name, _blend_equation_map[i].name) == 0)
				return _blend_equation_map[i].value;
		}

		return BlendEquation::COUNT;
	}

	static CullMode::Enum name_to_cull_mode(const char *name)
	{
		for (u32 i = 0; i < countof(_cull_mode_map); ++i) {
			if (strcmp(name, _cull_mode_map[i].name) == 0)
				return _cull_mode_map[i].value;
		}

		return CullMode::COUNT;
	}

	static PrimitiveType::Enum name_to_primitive_type(const char *name)
	{
		for (u32 i = 0; i < countof(_primitive_type_map); ++i) {
			if (strcmp(name, _primitive_type_map[i].name) == 0)
				return _primitive_type_map[i].value;
		}

		return PrimitiveType::COUNT;
	}

	static SamplerFilter::Enum name_to_sampler_filter(const char *name)
	{
		for (u32 i = 0; i < countof(_sampler_filter_map); ++i) {
			if (strcmp(name, _sampler_filter_map[i].name) == 0)
				return _sampler_filter_map[i].value;
		}

		return SamplerFilter::COUNT;
	}

	static SamplerWrap::Enum name_to_sampler_wrap(const char *name)
	{
		for (u32 i = 0; i < countof(_sampler_wrap_map); ++i) {
			if (strcmp(name, _sampler_wrap_map[i].name) == 0)
				return _sampler_wrap_map[i].value;
		}

		return SamplerWrap::COUNT;
	}

	struct FunctionOp
	{
		enum Enum
		{
			OP_LOGIC_AND,
			OP_LOGIC_OR,
			OP_LOGIC_NOT,
			OP_DEFINED,

			COUNT
		};
	};

	const char *function_names[] =
	{
		"&&",
		"||",
		"!",
		"defined"
	};
	CE_STATIC_ASSERT(countof(function_names) == FunctionOp::COUNT);

	expression_language::Function function_values[] =
	{
		{ FunctionOp::OP_LOGIC_AND,  5, 2 },
		{ FunctionOp::OP_LOGIC_OR,   4, 2 },
		{ FunctionOp::OP_LOGIC_NOT, 16, 1 },
		{ FunctionOp::OP_DEFINED,   17, 1 }
	};
	CE_STATIC_ASSERT(countof(function_values) == FunctionOp::COUNT);

	static s32 run_external_compiler(Process &pr
		, const char *shaderc
		, const char *infile
		, const char *outfile
		, const char *varying
		, const char *type
		, const char *platform
		, const Vector<DynamicString> &defines
		)
	{
		Array<const char *> argv(default_allocator());
		array::push_back(argv, shaderc);
		array::push_back(argv, (const char *)"-f");
		array::push_back(argv, infile);
		array::push_back(argv, (const char *)"-o");
		array::push_back(argv, outfile);
		array::push_back(argv, (const char *)"--varyingdef");
		array::push_back(argv, varying);
		array::push_back(argv, (const char *)"--type");
		array::push_back(argv, type);
		array::push_back(argv, (const char *)"--platform");
		array::push_back(argv, platform);

		StringStream defines_string(default_allocator());
		for (u32 i = 0; i < vector::size(defines); ++i)
			defines_string << defines[i].c_str() << ";";

		if (array::size(defines_string) > 0) {
			array::push_back(argv, (const char *)"--define");
			array::push_back(argv, string_stream::c_str(defines_string));
		}

		if (strcmp(platform, "android") == 0 || strcmp(platform, "asm.js") == 0) {
			array::push_back(argv, (const char *)"--profile");
			array::push_back(argv, (const char *)"300_es"); // GLES
		} else if (strcmp(platform, "linux") == 0) {
			array::push_back(argv, (const char *)"--profile");
			array::push_back(argv, (const char *)"150"); // OpenGL 3.2+
		} else if (strcmp(platform, "windows") == 0) {
			array::push_back(argv, (const char *)"--profile");
			array::push_back(argv, (const char *)"s_4_0");
		} else {
			return -1;
		}

		array::push_back(argv, (const char *)NULL);
		return pr.spawn(array::begin(argv), CROWN_PROCESS_STDOUT_PIPE | CROWN_PROCESS_STDERR_MERGE);
	}

	struct RenderState
	{
		ALLOCATOR_AWARE;

		struct State
		{
			Option<bool> _rgb_write_enable;
			Option<bool> _alpha_write_enable;
			Option<bool> _depth_write_enable;
			Option<bool> _depth_enable;
			Option<bool> _blend_enable;
			Option<DepthFunction::Enum> _depth_func;
			Option<BlendFunction::Enum> _blend_src;
			Option<BlendFunction::Enum> _blend_dst;
			Option<BlendEquation::Enum> _blend_equation;
			Option<CullMode::Enum> _cull_mode;
			Option<PrimitiveType::Enum> _primitive_type;

			void dump()
			{
				logi(SHADER_RESOURCE, "rgb_write_enable %d", _rgb_write_enable.value());
				logi(SHADER_RESOURCE, "alpha_write_enable %d", _alpha_write_enable.value());
				logi(SHADER_RESOURCE, "depth_write_enable %d", _depth_write_enable.value());
				logi(SHADER_RESOURCE, "depth_enable %d", _depth_enable.value());
				logi(SHADER_RESOURCE, "blend_enable %d", _blend_enable.value());
				logi(SHADER_RESOURCE, "depth_func %d", _depth_func.value());
				logi(SHADER_RESOURCE, "blend_src %d", _blend_src.value());
				logi(SHADER_RESOURCE, "blend_dst %d", _blend_dst.value());
				logi(SHADER_RESOURCE, "blend_equation %d", _blend_equation.value());
				logi(SHADER_RESOURCE, "cull_mode %d", _cull_mode.value());
				logi(SHADER_RESOURCE, "primitive_type %d", _primitive_type.value());
			}

			State()
				: _rgb_write_enable(true)
				, _alpha_write_enable(true)
				, _depth_write_enable(true)
				, _depth_enable(true)
				, _blend_enable(false)
				, _depth_func(DepthFunction::LEQUAL)
				, _blend_src(BlendFunction::SRC_ALPHA)
				, _blend_dst(BlendFunction::INV_SRC_ALPHA)
				, _blend_equation(BlendEquation::ADD)
				, _cull_mode(CullMode::CW)
				, _primitive_type(PrimitiveType::PT_TRIANGLES)
			{
			}

			State(const State &other) = delete;

			State &operator=(const State &other) = delete;

			void overwrite_changed_properties(const State &other)
			{
				if (other._rgb_write_enable.has_changed())
					_rgb_write_enable.set_value(other._rgb_write_enable.value());
				if (other._alpha_write_enable.has_changed())
					_alpha_write_enable.set_value(other._alpha_write_enable.value());
				if (other._depth_write_enable.has_changed())
					_depth_write_enable.set_value(other._depth_write_enable.value());
				if (other._depth_enable.has_changed())
					_depth_enable.set_value(other._depth_enable.value());
				if (other._blend_enable.has_changed())
					_blend_enable.set_value(other._blend_enable.value());
				if (other._depth_func.has_changed())
					_depth_func.set_value(other._depth_func.value());
				if (other._blend_src.has_changed())
					_blend_src.set_value(other._blend_src.value());
				if (other._blend_dst.has_changed())
					_blend_dst.set_value(other._blend_dst.value());
				if (other._blend_equation.has_changed())
					_blend_equation.set_value(other._blend_equation.value());
				if (other._cull_mode.has_changed())
					_cull_mode.set_value(other._cull_mode.value());
				if (other._primitive_type.has_changed())
					_primitive_type.set_value(other._primitive_type.value());
			}

			u64 encode() const
			{
				const u64 depth_func = _depth_enable.value()
					? _bgfx_depth_func_map[_depth_func.value()]
					: 0
					;
				const u64 blend_func = _blend_enable.value() && _blend_src.value() != BlendFunction::COUNT && _blend_dst.value() != BlendFunction::COUNT
					? BGFX_STATE_BLEND_FUNC(_bgfx_blend_func_map[_blend_src.value()], _bgfx_blend_func_map[_blend_dst.value()])
					: 0
					;
				const u64 blend_eq = _blend_enable.value() && _blend_equation.value() != BlendEquation::COUNT
					? BGFX_STATE_BLEND_EQUATION(_bgfx_blend_equation_map[_blend_equation.value()])
					: 0
					;
				const u64 cull_mode = _cull_mode.value() != CullMode::COUNT
					? _bgfx_cull_mode_map[_cull_mode.value()]
					: 0
					;
				const u64 primitive_type = _primitive_type.value() != PrimitiveType::COUNT
					? _bgfx_primitive_type_map[_primitive_type.value()]
					: 0
					;

				u64 state = 0;
				state |= _rgb_write_enable.value()   ? BGFX_STATE_WRITE_RGB : 0;
				state |= _alpha_write_enable.value() ? BGFX_STATE_WRITE_A   : 0;
				state |= _depth_write_enable.value() ? BGFX_STATE_WRITE_Z   : 0;
				state |= depth_func;
				state |= blend_func;
				state |= blend_eq;
				state |= cull_mode;
				state |= primitive_type;

				return state;
			}
		};

		struct StateIndex
		{
			uintptr_t sort_key;
			u32 index;
		};

		DynamicString _inherit;
		Vector<DynamicString> _expressions;
		Array<State> _states;
		Array<StateIndex> _states_indices;

		explicit RenderState(Allocator &a)
			: _inherit(a)
			, _expressions(a)
			, _states(a)
			, _states_indices(a)
		{
		}

		void push_back_states(const DynamicString &expr, const RenderState::State &state, uintptr_t sort_key)
		{
			vector::push_back(_expressions, expr);
			u32 state_index = array::size(_states);
			array::push_back(_states, state);
			array::push_back(_states_indices, { sort_key, state_index });
			std::sort(array::begin(_states_indices)
				, array::end(_states_indices)
				, [](const StateIndex &a, const StateIndex &b) {
					return a.sort_key < b.sort_key;
				});
		}

		void push_back_states(const char *expr, const RenderState::State &state, uintptr_t sort_key)
		{
			DynamicString expr_str(default_allocator());
			expr_str = expr;
			push_back_states(expr_str, state, sort_key);
		}
	};

	struct SamplerState
	{
		SamplerFilter::Enum _filter_min;
		SamplerFilter::Enum _filter_mag;
		SamplerWrap::Enum _wrap_u;
		SamplerWrap::Enum _wrap_v;
		SamplerWrap::Enum _wrap_w;

		SamplerState()
		{
			reset();
		}

		void reset()
		{
			_filter_min = SamplerFilter::COUNT;
			_filter_mag = SamplerFilter::COUNT;
			_wrap_u = SamplerWrap::COUNT;
			_wrap_v = SamplerWrap::COUNT;
			_wrap_w = SamplerWrap::COUNT;
		}

		u32 encode() const
		{
			u32 state = 0;
			state |= _filter_min != SamplerFilter::COUNT ? _bgfx_sampler_filter_min_map[_filter_min] : 0;
			state |= _filter_mag != SamplerFilter::COUNT ? _bgfx_sampler_filter_mag_map[_filter_mag] : 0;
			state |= _wrap_u != SamplerWrap::COUNT ? _bgfx_sampler_wrap_u_map[_wrap_u] : 0;
			state |= _wrap_v != SamplerWrap::COUNT ? _bgfx_sampler_wrap_v_map[_wrap_v] : 0;
			state |= _wrap_w != SamplerWrap::COUNT ? _bgfx_sampler_wrap_w_map[_wrap_w] : 0;
			return state;
		}
	};

	struct BgfxShader
	{
		ALLOCATOR_AWARE;

		Vector<DynamicString> _includes;
		DynamicString _code;
		DynamicString _vs_code;
		DynamicString _fs_code;
		DynamicString _varying;
		DynamicString _vs_input_output;
		DynamicString _fs_input_output;
		HashMap<DynamicString, DynamicString> _samplers;

		explicit BgfxShader(Allocator &a)
			: _includes(a)
			, _code(a)
			, _vs_code(a)
			, _fs_code(a)
			, _varying(a)
			, _vs_input_output(a)
			, _fs_input_output(a)
			, _samplers(a)
		{
		}
	};

	struct ShaderPermutation
	{
		ALLOCATOR_AWARE;

		DynamicString _bgfx_shader;
		DynamicString _render_state;

		explicit ShaderPermutation(Allocator &a)
			: _bgfx_shader(a)
			, _render_state(a)
		{
		}
	};

	struct StaticCompile
	{
		ALLOCATOR_AWARE;

		DynamicString _shader;
		Vector<DynamicString> _defines;

		explicit StaticCompile(Allocator &a)
			: _shader(a)
			, _defines(a)
		{
		}
	};

	struct ShaderCompiler
	{
		CompileOptions &_opts;
		HashSet<DynamicString> _parsed_includes;
		HashMap<DynamicString, RenderState> _render_states;
		HashMap<DynamicString, SamplerState> _sampler_states;
		HashMap<DynamicString, BgfxShader> _bgfx_shaders;
		HashMap<DynamicString, ShaderPermutation> _shaders;
		Vector<StaticCompile> _static_compile;

		DynamicString _vs_src_path;
		DynamicString _fs_src_path;
		DynamicString _varying_path;
		DynamicString _vs_out_path;
		DynamicString _fs_out_path;

		explicit ShaderCompiler(CompileOptions &opts)
			: _opts(opts)
			, _parsed_includes(default_allocator())
			, _render_states(default_allocator())
			, _sampler_states(default_allocator())
			, _bgfx_shaders(default_allocator())
			, _shaders(default_allocator())
			, _static_compile(default_allocator())
			, _vs_src_path(default_allocator())
			, _fs_src_path(default_allocator())
			, _varying_path(default_allocator())
			, _vs_out_path(default_allocator())
			, _fs_out_path(default_allocator())
		{
			_opts.temporary_path(_vs_src_path, "vs_src.sc");
			_opts.temporary_path(_fs_src_path, "fs_src.sc");
			_opts.temporary_path(_varying_path, "varying.sc");
			_opts.temporary_path(_vs_out_path, "vs_out.bin");
			_opts.temporary_path(_fs_out_path, "fs_out.bin");
		}

		s32 parse(const char *path, bool is_include)
		{
			TempAllocator256 ta;
			DynamicString path_str(ta);
			path_str = path;

			if (hash_set::has(_parsed_includes, path_str))
				return 0;

			s32 err = parse(_opts.read(path), is_include);
			ENSURE_OR_RETURN(err == 0, _opts);
			hash_set::insert(_parsed_includes, path_str);
			return err;
		}

		s32 parse(Buffer buf, bool is_include)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, buf), _opts);

			if (json_object::has(obj, "include")) {
				JsonArray arr(ta);
				RETURN_IF_ERROR(sjson::parse_array(arr, obj["include"]), _opts);

				for (u32 i = 0; i < array::size(arr); ++i) {
					DynamicString path(ta);
					RETURN_IF_ERROR(sjson::parse_string(path, arr[i]), _opts);
					s32 err = parse(path.c_str(), true);
					ENSURE_OR_RETURN(err == 0, _opts);
				}
			}

			if (json_object::has(obj, "render_states")) {
				s32 err = parse_render_states(obj["render_states"]);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			if (json_object::has(obj, "sampler_states")) {
				s32 err = parse_sampler_states(obj["sampler_states"]);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			if (json_object::has(obj, "bgfx_shaders")) {
				s32 err = parse_bgfx_shaders(obj["bgfx_shaders"]);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			if (json_object::has(obj, "shaders")) {
				s32 err = parse_shaders(obj["shaders"]);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			if (!is_include) { // Do not merge static_compile entries.
				if (json_object::has(obj, "static_compile")) {
					s32 err = parse_static_compile(obj["static_compile"]);
					ENSURE_OR_RETURN(err == 0, _opts);
				}
			}

			return 0;
		}

		s32 parse_states_compat(RenderState::State &state, JsonObject &obj)
		{
			// gui = {
			//   ...
			//   states = { <-- You are here.
			//   }
			// }
			// This function is for backwards compatibility only.
			// See: parse_states() and parse_conditional_states().

			TempAllocator4096 ta;
			const char *warn_msg = "RenderState properties are deprecated. Use 'states' object instead.";

			if (json_object::has(obj, "rgb_write_enable")) {
				logw(SHADER_RESOURCE, warn_msg);
				bool enable = RETURN_IF_ERROR(sjson::parse_bool(obj["rgb_write_enable"]), _opts);
				state._rgb_write_enable.set_value(enable);
			}

			if (json_object::has(obj, "alpha_write_enable")) {
				logw(SHADER_RESOURCE, warn_msg);
				bool enable = RETURN_IF_ERROR(sjson::parse_bool(obj["alpha_write_enable"]), _opts);
				state._alpha_write_enable.set_value(enable);
			}

			if (json_object::has(obj, "depth_write_enable")) {
				logw(SHADER_RESOURCE, warn_msg);
				bool enable = RETURN_IF_ERROR(sjson::parse_bool(obj["depth_write_enable"]), _opts);
				state._depth_write_enable.set_value(enable);
			}

			if (json_object::has(obj, "depth_enable")) {
				logw(SHADER_RESOURCE, warn_msg);
				bool enable = RETURN_IF_ERROR(sjson::parse_bool(obj["depth_enable"]), _opts);
				state._depth_enable.set_value(enable);
			}

			if (json_object::has(obj, "blend_enable")) {
				logw(SHADER_RESOURCE, warn_msg);
				bool enable = RETURN_IF_ERROR(sjson::parse_bool(obj["blend_enable"]), _opts);
				state._blend_enable.set_value(enable);
			}

			if (json_object::has(obj, "depth_func")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString depth_func(ta);
				RETURN_IF_ERROR(sjson::parse_string(depth_func, obj["depth_func"]), _opts);
				state._depth_func.set_value(name_to_depth_func(depth_func.c_str()));
				RETURN_IF_FALSE(state._depth_func.value() != DepthFunction::COUNT
					, _opts
					, "Unknown depth test: '%s'"
					, depth_func.c_str()
					);
			}

			if (json_object::has(obj, "blend_src")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString blend_src(ta);
				RETURN_IF_ERROR(sjson::parse_string(blend_src, obj["blend_src"]), _opts);
				state._blend_src.set_value(name_to_blend_function(blend_src.c_str()));
				RETURN_IF_FALSE(state._blend_src.value() != BlendFunction::COUNT
					, _opts
					, "Unknown blend function: '%s'"
					, blend_src.c_str()
					);
			}

			if (json_object::has(obj, "blend_dst")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString blend_dst(ta);
				RETURN_IF_ERROR(sjson::parse_string(blend_dst, obj["blend_dst"]), _opts);
				state._blend_dst.set_value(name_to_blend_function(blend_dst.c_str()));
				RETURN_IF_FALSE(state._blend_dst.value() != BlendFunction::COUNT
					, _opts
					, "Unknown blend function: '%s'"
					, blend_dst.c_str()
					);
			}

			if (json_object::has(obj, "blend_equation")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString blend_equation(ta);
				RETURN_IF_ERROR(sjson::parse_string(blend_equation, obj["blend_equation"]), _opts);
				state._blend_equation.set_value(name_to_blend_equation(blend_equation.c_str()));
				RETURN_IF_FALSE(state._blend_equation.value() != BlendEquation::COUNT
					, _opts
					, "Unknown blend equation: '%s'"
					, blend_equation.c_str()
					);
			}

			if (json_object::has(obj, "cull_mode")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString cull_mode(ta);
				RETURN_IF_ERROR(sjson::parse_string(cull_mode, obj["cull_mode"]), _opts);
				state._cull_mode.set_value(name_to_cull_mode(cull_mode.c_str()));
				RETURN_IF_FALSE(state._cull_mode.value() != CullMode::COUNT
					, _opts
					, "Unknown cull mode: '%s'"
					, cull_mode.c_str()
					);
			}

			if (json_object::has(obj, "primitive_type")) {
				logw(SHADER_RESOURCE, warn_msg);
				DynamicString primitive_type(ta);
				RETURN_IF_ERROR(sjson::parse_string(primitive_type, obj["primitive_type"]), _opts);
				state._primitive_type.set_value(name_to_primitive_type(primitive_type.c_str()));
				RETURN_IF_FALSE(state._primitive_type.value() != PrimitiveType::COUNT
					, _opts
					, "Unknown primitive type: '%s'"
					, primitive_type.c_str()
					);
			}

			return 0;
		}

		s32 parse_states(RenderState::State &state, const char *json)
		{
			// gui = {
			//   states = { <-- You are here.
			//   }
			//   ...
			// }
			//
			// Possible cases:
			// a) "some_expr" = { state_a = b, state_b = c, ... }
			//   Ignore, this is handled by parse_conditional_states().
			// b) state_x = y

			TempAllocator4096 ta;
			JsonObject states(ta);
			RETURN_IF_ERROR(sjson::parse_object(states, json), _opts);

			auto cur = json_object::begin(states);
			auto end = json_object::end(states);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(states, cur);

				// It must be a regular key/value state.
				if (cur->first == "rgb_write_enable") {
					bool enable = RETURN_IF_ERROR(sjson::parse_bool(states["rgb_write_enable"]), _opts);
					state._rgb_write_enable.set_value(enable);
				} else if (cur->first == "alpha_write_enable") {
					bool enable = RETURN_IF_ERROR(sjson::parse_bool(states["alpha_write_enable"]), _opts);
					state._alpha_write_enable.set_value(enable);
				} else if (cur->first == "depth_write_enable") {
					bool enable = RETURN_IF_ERROR(sjson::parse_bool(states["depth_write_enable"]), _opts);
					state._depth_write_enable.set_value(enable);
				} else if (cur->first == "depth_enable") {
					bool enable = RETURN_IF_ERROR(sjson::parse_bool(states["depth_enable"]), _opts);
					state._depth_enable.set_value(enable);
				} else if (cur->first == "blend_enable") {
					bool enable = RETURN_IF_ERROR(sjson::parse_bool(states["blend_enable"]), _opts);
					state._blend_enable.set_value(enable);
				} else if (cur->first == "depth_func") {
					DynamicString depth_func(ta);
					RETURN_IF_ERROR(sjson::parse_string(depth_func, states["depth_func"]), _opts);
					state._depth_func.set_value(name_to_depth_func(depth_func.c_str()));
					RETURN_IF_FALSE(state._depth_func.value() != DepthFunction::COUNT
						, _opts
						, "Unknown depth test: '%s'"
						, depth_func.c_str()
						);
				} else if (cur->first == "blend_src") {
					DynamicString blend_src(ta);
					RETURN_IF_ERROR(sjson::parse_string(blend_src, states["blend_src"]), _opts);
					state._blend_src.set_value(name_to_blend_function(blend_src.c_str()));
					RETURN_IF_FALSE(state._blend_src.value() != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s'"
						, blend_src.c_str()
						);
				} else if (cur->first == "blend_dst") {
					DynamicString blend_dst(ta);
					RETURN_IF_ERROR(sjson::parse_string(blend_dst, states["blend_dst"]), _opts);
					state._blend_dst.set_value(name_to_blend_function(blend_dst.c_str()));
					RETURN_IF_FALSE(state._blend_dst.value() != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s'"
						, blend_dst.c_str()
						);
				} else if (cur->first == "blend_equation") {
					DynamicString blend_equation(ta);
					RETURN_IF_ERROR(sjson::parse_string(blend_equation, states["blend_equation"]), _opts);
					state._blend_equation.set_value(name_to_blend_equation(blend_equation.c_str()));
					RETURN_IF_FALSE(state._blend_equation.value() != BlendEquation::COUNT
						, _opts
						, "Unknown blend equation: '%s'"
						, blend_equation.c_str()
						);
				} else if (cur->first == "cull_mode") {
					DynamicString cull_mode(ta);
					RETURN_IF_ERROR(sjson::parse_string(cull_mode, states["cull_mode"]), _opts);
					state._cull_mode.set_value(name_to_cull_mode(cull_mode.c_str()));
					RETURN_IF_FALSE(state._cull_mode.value() != CullMode::COUNT
						, _opts
						, "Unknown cull mode: '%s'"
						, cull_mode.c_str()
						);
				} else if (cur->first == "primitive_type") {
					DynamicString primitive_type(ta);
					RETURN_IF_ERROR(sjson::parse_string(primitive_type, states["primitive_type"]), _opts);
					state._primitive_type.set_value(name_to_primitive_type(primitive_type.c_str()));
					RETURN_IF_FALSE(state._primitive_type.value() != PrimitiveType::COUNT
						, _opts
						, "Unknown primitive type: '%s'"
						, primitive_type.c_str()
						);
				} else {
					// Skip conditionals state objects, error out on anything else.
					if (sjson::type(cur->second) != JsonValueType::OBJECT) {
						RETURN_IF_FALSE(false
							, _opts
							, "Unknown state property: '%.*s'"
							, cur->first.length()
							, cur->first.data()
							);
					}
				}
			}

			return 0;
		}

		s32 parse_conditional_states(RenderState &rs, const char *json)
		{
			// gui = {
			//   states = { <-- You are here.
			//   }
			//   ...
			// }
			//
			// Possible cases:
			// a) "some_expr" = { state_a = b, state_b = c, ... }
			// b) state_x = y
			//   Ignore, this is handled by parse_states().

			TempAllocator4096 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse_object(obj, json), _opts);

			// Read conditional states.
			auto cur = json_object::begin(obj);
			auto end = json_object::end(obj);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(obj, cur);

				if (sjson::type(cur->second) == JsonValueType::OBJECT) {
					// It must be a conditional expression: "expr()" = { ... }
					RenderState::State states;
					s32 err = parse_states(states, cur->second);
					ENSURE_OR_RETURN(err == 0, _opts);

					DynamicString expr(default_allocator());
					expr = cur->first;
					// Use cur->first.data() as sort key so we can sort states
					// by the order they appear in the source file.
					rs.push_back_states(expr
						, states
						, (uintptr_t)(void *)cur->first.data()
						);
				}
			}

			return 0;
		}

		s32 parse_render_state(RenderState &rs, const char *json)
		{
			// gui = { <-- You are here.
			//   states = {
			//   }
			//   ...
			// }

			TempAllocator4096 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse_object(obj, json), _opts);
			s32 err = 0;

			// Read inherit render state if any.
			if (json_object::has(obj, "inherit")) {
				RETURN_IF_ERROR(sjson::parse_string(rs._inherit, obj["inherit"]), _opts);
			}

			// Read states from render state object itself; for backwards compatibility.
			RenderState::State states_compat;
			err = parse_states_compat(states_compat, obj);
			ENSURE_OR_RETURN(err == 0, _opts);
			rs.push_back_states("1", states_compat, 0u); // Always applied.

			// Read states from new, dedicated "states" object.
			if (json_object::has(obj, "states")) {
				RenderState::State states;
				err = parse_states(states, obj["states"]);
				ENSURE_OR_RETURN(err == 0, _opts);
				rs.push_back_states("1", states, 1u); // Always applied.

				err = parse_conditional_states(rs, obj["states"]);
			}

			return 0;
		}

		s32 parse_render_states(const char *json)
		{
			TempAllocator4096 ta;
			JsonObject render_states(ta);
			RETURN_IF_ERROR(sjson::parse_object(render_states, json), _opts);

			auto cur = json_object::begin(render_states);
			auto end = json_object::end(render_states);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(render_states, cur);

				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, cur->second), _opts);

				RenderState rs(default_allocator());
				s32 err = parse_render_state(rs, cur->second);
				ENSURE_OR_RETURN(err == 0, _opts);

				DynamicString key(ta);
				key = cur->first;

				RETURN_IF_FALSE(!hash_map::has(_render_states, key)
					, _opts
					, "Render state redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_render_states, key, rs);
			}

			return 0;
		}

		s32 parse_sampler_states(const char *json)
		{
			TempAllocator4096 ta;
			JsonObject sampler_states(ta);
			RETURN_IF_ERROR(sjson::parse_object(sampler_states, json), _opts);

			auto cur = json_object::begin(sampler_states);
			auto end = json_object::end(sampler_states);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(sampler_states, cur);

				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, cur->second), _opts);

				const bool has_filter_min = json_object::has(obj, "filter_min");
				const bool has_filter_mag = json_object::has(obj, "filter_mag");
				const bool has_wrap_u = json_object::has(obj, "wrap_u");
				const bool has_wrap_v = json_object::has(obj, "wrap_v");
				const bool has_wrap_w = json_object::has(obj, "wrap_w");

				SamplerState ss;
				ss.reset();

				DynamicString filter_min(ta);
				DynamicString filter_mag(ta);
				DynamicString wrap_u(ta);
				DynamicString wrap_v(ta);
				DynamicString wrap_w(ta);

				if (has_filter_min) {
					RETURN_IF_ERROR(sjson::parse_string(filter_min, obj["filter_min"]), _opts);
					ss._filter_min = name_to_sampler_filter(filter_min.c_str());
					RETURN_IF_FALSE(ss._filter_min != SamplerFilter::COUNT
						, _opts
						, "Unknown sampler filter: '%s'"
						, filter_min.c_str()
						);
				}

				if (has_filter_mag) {
					RETURN_IF_ERROR(sjson::parse_string(filter_mag, obj["filter_mag"]), _opts);
					ss._filter_mag = name_to_sampler_filter(filter_mag.c_str());
					RETURN_IF_FALSE(ss._filter_mag != SamplerFilter::COUNT
						, _opts
						, "Unknown sampler filter: '%s'"
						, filter_mag.c_str()
						);
				}

				if (has_wrap_u) {
					RETURN_IF_ERROR(sjson::parse_string(wrap_u, obj["wrap_u"]), _opts);
					ss._wrap_u = name_to_sampler_wrap(wrap_u.c_str());
					RETURN_IF_FALSE(ss._wrap_u != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_u.c_str()
						);
				}

				if (has_wrap_v) {
					RETURN_IF_ERROR(sjson::parse_string(wrap_v, obj["wrap_v"]), _opts);
					ss._wrap_v = name_to_sampler_wrap(wrap_v.c_str());
					RETURN_IF_FALSE(ss._wrap_v != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_v.c_str()
						);
				}

				if (has_wrap_w) {
					RETURN_IF_ERROR(sjson::parse_string(wrap_w, obj["wrap_w"]), _opts);
					ss._wrap_w = name_to_sampler_wrap(wrap_w.c_str());
					RETURN_IF_FALSE(ss._wrap_w != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_w.c_str()
						);
				}

				DynamicString key(ta);
				key = cur->first;

				RETURN_IF_FALSE(!hash_map::has(_sampler_states, key)
					, _opts
					, "Sampler state redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_sampler_states, key, ss);
			}

			return 0;
		}

		s32 parse_bgfx_shaders(const char *json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_shaders(ta);
			RETURN_IF_ERROR(sjson::parse_object(bgfx_shaders, json), _opts);

			auto cur = json_object::begin(bgfx_shaders);
			auto end = json_object::end(bgfx_shaders);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(bgfx_shaders, cur);

				JsonObject shader(ta);
				RETURN_IF_ERROR(sjson::parse_object(shader, cur->second), _opts);

				BgfxShader bgfxshader(default_allocator());
				if (json_object::has(shader, "includes")) {
					s32 err = parse_bgfx_includes(bgfxshader, shader["includes"]);
					ENSURE_OR_RETURN(err == 0, _opts);
				}
				if (json_object::has(shader, "code")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._code, shader["code"]), _opts);
				}
				if (json_object::has(shader, "vs_code")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._vs_code, shader["vs_code"]), _opts);
				}
				if (json_object::has(shader, "fs_code")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._fs_code, shader["fs_code"]), _opts);
				}
				if (json_object::has(shader, "varying")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._varying, shader["varying"]), _opts);
				}
				if (json_object::has(shader, "vs_input_output")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._vs_input_output, shader["vs_input_output"]), _opts);
				}
				if (json_object::has(shader, "fs_input_output")) {
					RETURN_IF_ERROR(sjson::parse_verbatim(bgfxshader._fs_input_output, shader["fs_input_output"]), _opts);
				}
				if (json_object::has(shader, "samplers")) {
					s32 err = parse_bgfx_samplers(bgfxshader, shader["samplers"]);
					ENSURE_OR_RETURN(err == 0, _opts);
				}

				DynamicString key(ta);
				key = cur->first;

				RETURN_IF_FALSE(!hash_map::has(_bgfx_shaders, key)
					, _opts
					, "Bgfx shader redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_bgfx_shaders, key, bgfxshader);
			}

			return 0;
		}

		s32 parse_bgfx_samplers(BgfxShader &bgfxshader, const char *json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_samplers(ta);
			RETURN_IF_ERROR(sjson::parse_object(bgfx_samplers, json), _opts);

			auto cur = json_object::begin(bgfx_samplers);
			auto end = json_object::end(bgfx_samplers);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(bgfx_samplers, cur);

				JsonObject sampler(ta);
				RETURN_IF_ERROR(sjson::parse_object(sampler, cur->second), _opts);

				DynamicString sampler_state(ta);
				RETURN_IF_ERROR(sjson::parse_string(sampler_state, sampler["sampler_state"]), _opts);

				RETURN_IF_FALSE(hash_map::has(_sampler_states, sampler_state)
					, _opts
					, "Unknown sampler state: '%s'"
					, sampler_state.c_str()
					);

				DynamicString key(ta);
				key = cur->first;

				RETURN_IF_FALSE(!hash_map::has(bgfxshader._samplers, key)
					, _opts
					, "Bgfx sampler redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(bgfxshader._samplers, key, sampler_state);
			}

			return 0;
		}

		s32 parse_bgfx_includes(BgfxShader &bgfxshader, const char *json)
		{
			if (json::type(json) == JsonValueType::STRING) {
				TempAllocator256 ta;
				DynamicString inc(ta);
				RETURN_IF_ERROR(sjson::parse_string(inc, json), _opts);
				vector::push_back(bgfxshader._includes, inc);
				logw(SHADER_RESOURCE, "'includes' string format is deprecated. Use array-of-strings format instead.");
				return 0;
			} else if (json::type(json) == JsonValueType::ARRAY) {
				TempAllocator1024 ta;
				JsonArray includes(ta);

				RETURN_IF_ERROR(sjson::parse_array(includes, json), _opts);

				for (u32 i = 0; i < array::size(includes); ++i) {
					DynamicString inc(ta);
					RETURN_IF_ERROR(sjson::parse_string(inc, includes[i]), _opts);
					vector::push_back(bgfxshader._includes, inc);
				}
				return 0;
			}

			RETURN_IF_FALSE(false, _opts, "'includes' must be either an array-of-strings or a string");
		}

		s32 parse_shaders(const char *json)
		{
			TempAllocator4096 ta;
			JsonObject shaders(ta);
			RETURN_IF_ERROR(sjson::parse_object(shaders, json), _opts);

			auto cur = json_object::begin(shaders);
			auto end = json_object::end(shaders);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(shaders, cur);

				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, cur->second), _opts);

				ShaderPermutation shader(default_allocator());
				RETURN_IF_ERROR(sjson::parse_string(shader._bgfx_shader, obj["bgfx_shader"]), _opts);
				RETURN_IF_ERROR(sjson::parse_string(shader._render_state, obj["render_state"]), _opts);

				DynamicString key(ta);
				key = cur->first;

				RETURN_IF_FALSE(!hash_map::has(_shaders, key)
					, _opts
					, "Shader redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_shaders, key, shader);
			}

			return 0;
		}

		s32 parse_static_compile(const char *json)
		{
			TempAllocator4096 ta;
			JsonArray static_compile(ta);
			RETURN_IF_ERROR(sjson::parse_array(static_compile, json), _opts);

			for (u32 ii = 0; ii < array::size(static_compile); ++ii) {
				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, static_compile[ii]), _opts);

				StaticCompile sc(default_allocator());
				RETURN_IF_ERROR(sjson::parse_string(sc._shader, obj["shader"]), _opts);

				JsonArray defines(ta);
				RETURN_IF_ERROR(sjson::parse_array(defines, obj["defines"]), _opts);
				for (u32 jj = 0; jj < array::size(defines); ++jj) {
					DynamicString def(ta);
					RETURN_IF_ERROR(sjson::parse_string(def, defines[jj]), _opts);
					vector::push_back(sc._defines, def);
				}

				vector::push_back(_static_compile, sc);
			}

			return 0;
		}

		void delete_temp_files()
		{
			_opts.delete_file(_vs_src_path.c_str());
			_opts.delete_file(_fs_src_path.c_str());
			_opts.delete_file(_varying_path.c_str());
			_opts.delete_file(_vs_out_path.c_str());
			_opts.delete_file(_fs_out_path.c_str());
		}

		s32 compile()
		{
			_opts.write(RESOURCE_HEADER(RESOURCE_VERSION_SHADER));
			_opts.write(vector::size(_static_compile));

			for (u32 ii = 0; ii < vector::size(_static_compile); ++ii) {
				const StaticCompile &sc              = _static_compile[ii];
				const DynamicString &shader          = sc._shader;
				const Vector<DynamicString> &defines = sc._defines;

				TempAllocator1024 ta;
				DynamicString str(ta);
				str = shader;
				for (u32 jj = 0; jj < vector::size(defines); ++jj) {
					str += "+";
					str += defines[jj];
				}
				const StringId32 shader_name(str.c_str());

				RETURN_IF_FALSE(hash_map::has(_shaders, sc._shader)
					, _opts
					, "Unknown shader: '%s'"
					, shader.c_str()
					);
				const ShaderPermutation sp_default(default_allocator());
				const ShaderPermutation &sp       = hash_map::get(_shaders, shader, sp_default);
				const DynamicString &bgfx_shader  = sp._bgfx_shader;
				const DynamicString &render_state = sp._render_state;

				RETURN_IF_FALSE(hash_map::has(_bgfx_shaders, sp._bgfx_shader)
					, _opts
					, "Unknown bgfx shader: '%s'"
					, bgfx_shader.c_str()
					);
				RETURN_IF_FALSE(hash_map::has(_render_states, sp._render_state)
					, _opts
					, "Unknown render state: '%s'"
					, render_state.c_str()
					);

				s32 err = 0;
				RenderState::State state;
				err = compile_render_state(state, render_state.c_str(), defines);
				ENSURE_OR_RETURN(err == 0, _opts);

				_opts.write(shader_name._id);                            // Shader name
				_opts.write(state.encode());                             // Render state
				compile_sampler_states(bgfx_shader.c_str());             // Sampler states
				err = compile_bgfx_shader(bgfx_shader.c_str(), defines); // Shader code
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			return 0;
		}

		void compile_sampler_states(const char *bgfx_shader)
		{
			TempAllocator512 ta;
			DynamicString key(ta);
			key = bgfx_shader;
			const BgfxShader shader_default(default_allocator());
			const BgfxShader &shader = hash_map::get(_bgfx_shaders, key, shader_default);

			_opts.write(hash_map::size(shader._samplers));

			auto cur = hash_map::begin(shader._samplers);
			auto end = hash_map::end(shader._samplers);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(shader._samplers, cur);

				const DynamicString &name = cur->first;
				const DynamicString &sampler_state = cur->second;
				const SamplerState ss_default;
				const SamplerState &ss = hash_map::get(_sampler_states, sampler_state, ss_default);

				_opts.write(name.to_string_id());
				_opts.write(ss.encode());
			}
		}

		// Collects code from @a shader and its includes recursively.
		s32 bgfx_shader_collect_code(StringStream &code, const BgfxShader &shader)
		{
			for (u32 i = 0; i < vector::size(shader._includes); ++i) {
				const BgfxShader included_default(default_allocator());
				const BgfxShader &included = hash_map::get(_bgfx_shaders, shader._includes[i], included_default);
				s32 err = bgfx_shader_collect_code(code, included);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			code << shader._code.c_str();
			return 0;
		}

		s32 compile_bgfx_shader(const char *bgfx_shader, const Vector<DynamicString> &defines)
		{
			TempAllocator512 taa;
			DynamicString key(taa);
			key = bgfx_shader;
			const BgfxShader shader_default(default_allocator());
			const BgfxShader &shader = hash_map::get(_bgfx_shaders, key, shader_default);

			StringStream code(default_allocator());
			s32 err = bgfx_shader_collect_code(code, shader);
			ENSURE_OR_RETURN(err == 0, _opts);

			// Generate final shader code.
			StringStream varying_code(default_allocator());
			StringStream vs_code(default_allocator());
			StringStream fs_code(default_allocator());
			// Generate varying.
			varying_code << shader._varying.c_str();
			// Generate vertex shader.
			vs_code << shader._vs_input_output.c_str();
			vs_code << string_stream::c_str(code);
			vs_code << shader._vs_code.c_str();
			// Generate fragment shader.
			fs_code << shader._fs_input_output.c_str();
			fs_code << string_stream::c_str(code);
			fs_code << shader._fs_code.c_str();

			_opts.write_temporary(_vs_src_path.c_str(), vs_code);
			_opts.write_temporary(_fs_src_path.c_str(), fs_code);
			_opts.write_temporary(_varying_path.c_str(), varying_code);

			const char *shaderc = _opts.exe_path(shaderc_paths, countof(shaderc_paths));
			RETURN_IF_FALSE(shaderc != NULL, _opts, "shaderc not found");

			// Invoke shaderc
			Process pr_vert;
			Process pr_frag;
			s32 sc;

			sc = run_external_compiler(pr_vert
				, shaderc
				, _vs_src_path.c_str()
				, _vs_out_path.c_str()
				, _varying_path.c_str()
				, "vertex"
				, shaderc_platform[_opts._platform]
				, defines
				);
			if (sc != 0) {
				delete_temp_files();
				RETURN_IF_FALSE(sc == 0
					, _opts
					, "Failed to spawn `%s`"
					, shaderc
					);
			}

			sc = run_external_compiler(pr_frag
				, shaderc
				, _fs_src_path.c_str()
				, _fs_out_path.c_str()
				, _varying_path.c_str()
				, "fragment"
				, shaderc_platform[_opts._platform]
				, defines
				);
			if (sc != 0) {
				delete_temp_files();
				RETURN_IF_FALSE(sc == 0
					, _opts
					, "Failed to spawn `%s`"
					, shaderc
					);
			}

			// Check shaderc exit code
			s32 ec;
			TempAllocator4096 ta;
			StringStream output_vert(ta);
			StringStream output_frag(ta);

			_opts.read_output(output_vert, pr_vert);
			ec = pr_vert.wait();
			if (ec != 0) {
				pr_frag.wait();
				delete_temp_files();
				RETURN_IF_FALSE(false
					, _opts
					, "Failed to compile vertex shader `%s`:\n%s"
					, bgfx_shader
					, string_stream::c_str(output_vert)
					);
			}

			_opts.read_output(output_frag, pr_frag);
			ec = pr_frag.wait();
			if (ec != 0) {
				delete_temp_files();
				RETURN_IF_FALSE(false
					, _opts
					, "Failed to compile fragment shader `%s`:\n%s"
					, bgfx_shader
					, string_stream::c_str(output_frag)
					);
			}

			Buffer vs_data = _opts.read_temporary(_vs_out_path.c_str());
			Buffer fs_data = _opts.read_temporary(_fs_out_path.c_str());
			delete_temp_files();

			// Write
			_opts.write(array::size(vs_data));
			_opts.write(vs_data);
			_opts.write(array::size(fs_data));
			_opts.write(fs_data);

			return 0;
		}

		static void compute_function(int op_code, expression_language::Stack &stack)
		{
#define POP() expression_language::pop(stack)
#define PUSH(f) expression_language::push(stack, f)

			float a, b;

			switch (op_code) {
			case FunctionOp::OP_LOGIC_AND: b = POP(); a = POP(); PUSH(f32(a && b)); break;
			case FunctionOp::OP_LOGIC_OR: b = POP(); a = POP(); PUSH(f32(a || b)); break;
			case FunctionOp::OP_LOGIC_NOT: a = POP(); PUSH(f32(!a)); break;
			case FunctionOp::OP_DEFINED: a = POP(); PUSH(f32(a == 1.0)); break;
			default:
				CE_FATAL("Unknown opcode %d", op_code);
			}

#undef POP
#undef PUSH
		}

		static s32 eval(const char *expr, u32 num_constants, const char **constant_names, const f32 *constant_values, CompileOptions &opts)
		{
			namespace el = expression_language;

			el::CompileEnvironment env;
			env.num_variables = 0;
			env.variable_names = NULL;
			env.num_constants = num_constants;
			env.constant_names = constant_names;
			env.constant_values = constant_values;
			env.num_functions = countof(function_names);
			env.function_names = function_names;
			env.function_values = function_values;
			env.compute_function = compute_function;

			f32 stack_data[32];
			f32 variables[32];
			expression_language::Stack stack(stack_data, countof(stack_data));

			unsigned byte_code[1024];
			u32 num = expression_language::compile(expr
				, env
				, byte_code
				, countof(byte_code)
				);
			ENSURE_OR_RETURN(num <= countof(byte_code), opts);

			bool ok = expression_language::run(byte_code, variables, stack);
			RETURN_IF_FALSE(ok && stack.size > 0
				, opts
				, "Failed to evaluate expression: '%s'"
				, expr
				);
			return (bool)stack_data[stack.size - 1];
		}

		s32 compile_render_state(RenderState::State &state, const char *render_state, const Vector<DynamicString> &defines)
		{
			TempAllocator512 taa;
			DynamicString key(taa);
			key = render_state;
			const RenderState rs_default(default_allocator());
			const RenderState &rs = hash_map::get(_render_states, key, rs_default);

			// Compile inherited state if any.
			if (!(rs._inherit == "")) {
				RETURN_IF_FALSE(hash_map::has(_render_states, rs._inherit)
					, _opts
					, "Unknown inherit render state: '%s'"
					, rs._inherit.c_str()
					);

				s32 err = compile_render_state(state, rs._inherit.c_str(), defines);
				ENSURE_OR_RETURN(err == 0, _opts);
			}

			// Evaluate expressions and apply states.
			if (vector::size(rs._expressions) > 0) {
				// Convert defines to plain array.
				u32 num_constants = vector::size(defines);
				const char **constants = (const char **)default_allocator().allocate(num_constants * sizeof(char *));
				f32 *constant_values = (f32 *)default_allocator().allocate(num_constants * sizeof(f32 *));
				for (u32 i = 0; i < num_constants; ++i) {
					constants[i] = defines[i].c_str();
					constant_values[i] = 1.0f;
				}

				// Evaluate expressions in the same order they occur
				// in the source file.
				for (u32 i = 0; i < vector::size(rs._expressions); ++i) {
					const u32 state_index = rs._states_indices[i].index;
					const char *expr = rs._expressions[state_index].c_str();
					const RenderState::State &cond_state = rs._states[state_index];

					bool eval_result = eval(expr
						, num_constants
						, constants
						, constant_values
						, _opts
						);
					if (eval_result)
						state.overwrite_changed_properties(cond_state);
				}

				default_allocator().deallocate(constant_values);
				default_allocator().deallocate(constants);
			}

			return 0;
		}
	};

	s32 compile(CompileOptions &opts)
	{
		ShaderCompiler sc(opts);
		s32 err = sc.parse(opts.source_path(), false);
		ENSURE_OR_RETURN(err == 0, opts);

		return sc.compile();
	}

} // namespace shader_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
