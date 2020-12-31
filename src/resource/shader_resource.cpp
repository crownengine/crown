/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "device/device.h"
#include "resource/compile_options.inl"
#include "resource/resource_manager.h"
#include "resource/shader_resource.h"
#include "world/shader_manager.h"

namespace crown
{
namespace shader_resource_internal
{
	void* load(File& file, Allocator& a)
	{
		return device()->_shader_manager->load(file, a);
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		device()->_shader_manager->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		device()->_shader_manager->offline(id, rm);
	}

	void unload(Allocator& a, void* res)
	{
		device()->_shader_manager->unload(a, res);
	}

} // namespace shader_resource_internal

#if CROWN_CAN_COMPILE
namespace shader_resource_internal
{
	static const char* shaderc_paths[] =
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
		const char* name;
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
		const char* name;
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
		const char* name;
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
		const char* name;
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
		const char* name;
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
		const char* name;
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
		const char* name;
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
		BGFX_SAMPLER_MIN_POINT,       // SamplerFilter::POINT
		BGFX_SAMPLER_MIN_ANISOTROPIC  // SamplerFilter::ANISOTROPIC
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

	static DepthFunction::Enum name_to_depth_func(const char* name)
	{
		for (u32 i = 0; i < countof(_depth_test_map); ++i)
		{
			if (strcmp(name, _depth_test_map[i].name) == 0)
				return _depth_test_map[i].value;
		}

		return DepthFunction::COUNT;
	}

	static BlendFunction::Enum name_to_blend_function(const char* name)
	{
		for (u32 i = 0; i < countof(_blend_func_map); ++i)
		{
			if (strcmp(name, _blend_func_map[i].name) == 0)
				return _blend_func_map[i].value;
		}

		return BlendFunction::COUNT;
	}

	static BlendEquation::Enum name_to_blend_equation(const char* name)
	{
		for (u32 i = 0; i < countof(_blend_equation_map); ++i)
		{
			if (strcmp(name, _blend_equation_map[i].name) == 0)
				return _blend_equation_map[i].value;
		}

		return BlendEquation::COUNT;
	}

	static CullMode::Enum name_to_cull_mode(const char* name)
	{
		for (u32 i = 0; i < countof(_cull_mode_map); ++i)
		{
			if (strcmp(name, _cull_mode_map[i].name) == 0)
				return _cull_mode_map[i].value;
		}

		return CullMode::COUNT;
	}

	static PrimitiveType::Enum name_to_primitive_type(const char* name)
	{
		for (u32 i = 0; i < countof(_primitive_type_map); ++i)
		{
			if (strcmp(name, _primitive_type_map[i].name) == 0)
				return _primitive_type_map[i].value;
		}

		return PrimitiveType::COUNT;
	}

	static SamplerFilter::Enum name_to_sampler_filter(const char* name)
	{
		for (u32 i = 0; i < countof(_sampler_filter_map); ++i)
		{
			if (strcmp(name, _sampler_filter_map[i].name) == 0)
				return _sampler_filter_map[i].value;
		}

		return SamplerFilter::COUNT;
	}

	static SamplerWrap::Enum name_to_sampler_wrap(const char* name)
	{
		for (u32 i = 0; i < countof(_sampler_wrap_map); ++i)
		{
			if (strcmp(name, _sampler_wrap_map[i].name) == 0)
				return _sampler_wrap_map[i].value;
		}

		return SamplerWrap::COUNT;
	}

	static s32 run_external_compiler(Process& pr
		, const char* shaderc
		, const char* infile
		, const char* outfile
		, const char* varying
		, const char* type
		, const char* platform
		)
	{
		const char* argv[] =
		{
			shaderc,
			"-f",
			infile,
			"-o",
			outfile,
			"--varyingdef",
			varying,
			"--type",
			type,
			"--platform",
			platform,
			NULL,
			NULL,
			NULL,
		};

		if (strcmp(platform, "android") == 0)
		{
			argv[11] = "--profile";
			argv[12] = "1"; // GLES2
		}
		else if (strcmp(platform, "linux") == 0)
		{
			argv[11] = "--profile";
			argv[12] = "150"; // OpenGL 3.2+
		}
		else if (strcmp(platform, "windows") == 0)
		{
			argv[11] = "--profile";
			argv[12] = ((strcmp(type, "vertex") == 0) ? "vs_4_0" : "ps_4_0");
		}
		else
		{
			return -1;
		}

		return pr.spawn(argv, ProcessFlags::STDOUT_PIPE | ProcessFlags::STDERR_MERGE);
	}

	struct RenderState
	{
		bool _rgb_write_enable;
		bool _alpha_write_enable;
		bool _depth_write_enable;
		bool _depth_enable;
		bool _blend_enable;
		DepthFunction::Enum _depth_func;
		BlendFunction::Enum _blend_src;
		BlendFunction::Enum _blend_dst;
		BlendEquation::Enum _blend_equation;
		CullMode::Enum _cull_mode;
		PrimitiveType::Enum _primitive_type;

		RenderState()
		{
			reset();
		}

		void reset()
		{
			_rgb_write_enable = false;
			_alpha_write_enable = false;
			_depth_write_enable = false;
			_depth_enable = false;
			_blend_enable = false;
			_depth_func = DepthFunction::LEQUAL;
			_blend_src = BlendFunction::SRC_ALPHA;
			_blend_dst = BlendFunction::INV_SRC_ALPHA;
			_blend_equation = BlendEquation::ADD;
			_cull_mode = CullMode::CW;
			_primitive_type = PrimitiveType::PT_TRIANGLES;
		}

		u64 encode() const
		{
			const u64 depth_func = _depth_enable
				? _bgfx_depth_func_map[_depth_func]
				: 0
				;
			const u64 blend_func = _blend_enable && _blend_src != BlendFunction::COUNT && _blend_dst != BlendFunction::COUNT
				? BGFX_STATE_BLEND_FUNC(_bgfx_blend_func_map[_blend_src], _bgfx_blend_func_map[_blend_dst])
				: 0
				;
			const u64 blend_eq = _blend_enable && _blend_equation != BlendEquation::COUNT
				? BGFX_STATE_BLEND_EQUATION(_bgfx_blend_equation_map[_blend_equation])
				: 0
				;
			const u64 cull_mode = _cull_mode != CullMode::COUNT
				? _bgfx_cull_mode_map[_cull_mode]
				: 0
				;
			const u64 primitive_type = _primitive_type != PrimitiveType::COUNT
				? _bgfx_primitive_type_map[_primitive_type]
				: 0
				;

			u64 state = 0;
			state |= _rgb_write_enable   ? BGFX_STATE_WRITE_RGB : 0;
			state |= _alpha_write_enable ? BGFX_STATE_WRITE_A   : 0;
			state |= _depth_write_enable ? BGFX_STATE_WRITE_Z   : 0;
			state |= depth_func;
			state |= blend_func;
			state |= blend_eq;
			state |= cull_mode;
			state |= primitive_type;

			return state;
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

		DynamicString _includes;
		DynamicString _code;
		DynamicString _vs_code;
		DynamicString _fs_code;
		DynamicString _varying;
		DynamicString _vs_input_output;
		DynamicString _fs_input_output;
		HashMap<DynamicString, DynamicString> _samplers;

		explicit BgfxShader(Allocator& a)
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

		explicit ShaderPermutation(Allocator& a)
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

		explicit StaticCompile(Allocator& a)
			: _shader(a)
			, _defines(a)
		{
		}
	};

	struct ShaderCompiler
	{
		CompileOptions& _opts;
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

		explicit ShaderCompiler(CompileOptions& opts)
			: _opts(opts)
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

		s32 parse(const char* path)
		{
			return parse(_opts.read(path));
		}

		s32 parse(Buffer buf)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, buf);

			if (json_object::has(obj, "include"))
			{
				JsonArray arr(ta);
				sjson::parse_array(arr, obj["include"]);

				for (u32 i = 0; i < array::size(arr); ++i)
				{
					DynamicString path(ta);
					sjson::parse_string(path, arr[i]);
					parse(path.c_str());
				}
			}

			if (json_object::has(obj, "render_states"))
			{
				s32 err = parse_render_states(obj["render_states"]);
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			if (json_object::has(obj, "sampler_states"))
			{
				s32 err = parse_sampler_states(obj["sampler_states"]);
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			if (json_object::has(obj, "bgfx_shaders"))
			{
				s32 err = parse_bgfx_shaders(obj["bgfx_shaders"]);
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			if (json_object::has(obj, "shaders"))
			{
				s32 err = parse_shaders(obj["shaders"]);
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			if (json_object::has(obj, "static_compile"))
			{
				s32 err = parse_static_compile(obj["static_compile"]);
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			return 0;
		}

		s32 parse_render_states(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject render_states(ta);
			sjson::parse_object(render_states, json);

			auto cur = json_object::begin(render_states);
			auto end = json_object::end(render_states);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(render_states, cur);

				JsonObject obj(ta);
				sjson::parse_object(obj, cur->second);

				const bool rgb_write_enable   = sjson::parse_bool(obj["rgb_write_enable"]);
				const bool alpha_write_enable = sjson::parse_bool(obj["alpha_write_enable"]);
				const bool depth_write_enable = sjson::parse_bool(obj["depth_write_enable"]);
				const bool depth_enable       = sjson::parse_bool(obj["depth_enable"]);
				const bool blend_enable       = sjson::parse_bool(obj["blend_enable"]);

				const bool has_depth_func     = json_object::has(obj, "depth_func");
				const bool has_blend_src      = json_object::has(obj, "blend_src");
				const bool has_blend_dst      = json_object::has(obj, "blend_dst");
				const bool has_blend_equation = json_object::has(obj, "blend_equation");
				const bool has_cull_mode      = json_object::has(obj, "cull_mode");
				const bool has_primitive_type = json_object::has(obj, "primitive_type");

				RenderState rs;
				rs.reset();
				rs._rgb_write_enable   = rgb_write_enable;
				rs._alpha_write_enable = alpha_write_enable;
				rs._depth_write_enable = depth_write_enable;
				rs._depth_enable       = depth_enable;
				rs._blend_enable       = blend_enable;

				DynamicString depth_func(ta);
				DynamicString blend_src(ta);
				DynamicString blend_dst(ta);
				DynamicString blend_equation(ta);
				DynamicString cull_mode(ta);
				DynamicString primitive_type(ta);

				if (has_depth_func)
				{
					sjson::parse_string(depth_func, obj["depth_func"]);
					rs._depth_func = name_to_depth_func(depth_func.c_str());
					DATA_COMPILER_ASSERT(rs._depth_func != DepthFunction::COUNT
						, _opts
						, "Unknown depth test: '%s'"
						, depth_func.c_str()
						);
				}

				if (has_blend_src)
				{
					sjson::parse_string(blend_src, obj["blend_src"]);
					rs._blend_src = name_to_blend_function(blend_src.c_str());
					DATA_COMPILER_ASSERT(rs._blend_src != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s'"
						, blend_src.c_str()
						);
				}

				if (has_blend_dst)
				{
					sjson::parse_string(blend_dst, obj["blend_dst"]);
					rs._blend_dst = name_to_blend_function(blend_dst.c_str());
					DATA_COMPILER_ASSERT(rs._blend_dst != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s'"
						, blend_dst.c_str()
						);
				}

				if (has_blend_equation)
				{
					sjson::parse_string(blend_equation, obj["blend_equation"]);
					rs._blend_equation = name_to_blend_equation(blend_equation.c_str());
					DATA_COMPILER_ASSERT(rs._blend_equation != BlendEquation::COUNT
						, _opts
						, "Unknown blend equation: '%s'"
						, blend_equation.c_str()
						);
				}

				if (has_cull_mode)
				{
					sjson::parse_string(cull_mode, obj["cull_mode"]);
					rs._cull_mode = name_to_cull_mode(cull_mode.c_str());
					DATA_COMPILER_ASSERT(rs._cull_mode != CullMode::COUNT
						, _opts
						, "Unknown cull mode: '%s'"
						, cull_mode.c_str()
						);
				}

				if (has_primitive_type)
				{
					sjson::parse_string(primitive_type, obj["primitive_type"]);
					rs._primitive_type = name_to_primitive_type(primitive_type.c_str());
					DATA_COMPILER_ASSERT(rs._primitive_type != PrimitiveType::COUNT
						, _opts
						, "Unknown primitive type: '%s'"
						, primitive_type.c_str()
						);
				}

				DynamicString key(ta);
				key = cur->first;

				DATA_COMPILER_ASSERT(!hash_map::has(_render_states, key)
					, _opts
					, "Render state redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_render_states, key, rs);
			}

			return 0;
		}

		s32 parse_sampler_states(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject sampler_states(ta);
			sjson::parse_object(sampler_states, json);

			auto cur = json_object::begin(sampler_states);
			auto end = json_object::end(sampler_states);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(sampler_states, cur);

				JsonObject obj(ta);
				sjson::parse_object(obj, cur->second);

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

				if (has_filter_min)
				{
					sjson::parse_string(filter_min, obj["filter_min"]);
					ss._filter_min = name_to_sampler_filter(filter_min.c_str());
					DATA_COMPILER_ASSERT(ss._filter_min != SamplerFilter::COUNT
						, _opts
						, "Unknown sampler filter: '%s'"
						, filter_min.c_str()
						);
				}

				if (has_filter_mag)
				{
					sjson::parse_string(filter_mag, obj["filter_mag"]);
					ss._filter_mag = name_to_sampler_filter(filter_mag.c_str());
					DATA_COMPILER_ASSERT(ss._filter_mag != SamplerFilter::COUNT
						, _opts
						, "Unknown sampler filter: '%s'"
						, filter_mag.c_str()
						);
				}

				if (has_wrap_u)
				{
					sjson::parse_string(wrap_u, obj["wrap_u"]);
					ss._wrap_u = name_to_sampler_wrap(wrap_u.c_str());
					DATA_COMPILER_ASSERT(ss._wrap_u != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_u.c_str()
						);
				}

				if (has_wrap_v)
				{
					sjson::parse_string(wrap_v, obj["wrap_v"]);
					ss._wrap_v = name_to_sampler_wrap(wrap_v.c_str());
					DATA_COMPILER_ASSERT(ss._wrap_v != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_v.c_str()
						);
				}

				if (has_wrap_w)
				{
					sjson::parse_string(wrap_w, obj["wrap_w"]);
					ss._wrap_w = name_to_sampler_wrap(wrap_w.c_str());
					DATA_COMPILER_ASSERT(ss._wrap_w != SamplerWrap::COUNT
						, _opts
						, "Unknown wrap mode: '%s'"
						, wrap_w.c_str()
						);
				}

				DynamicString key(ta);
				key = cur->first;

				DATA_COMPILER_ASSERT(!hash_map::has(_sampler_states, key)
					, _opts
					, "Sampler state redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_sampler_states, key, ss);
			}

			return 0;
		}

		s32 parse_bgfx_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_shaders(ta);
			sjson::parse_object(bgfx_shaders, json);

			auto cur = json_object::begin(bgfx_shaders);
			auto end = json_object::end(bgfx_shaders);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(bgfx_shaders, cur);

				JsonObject shader(ta);
				sjson::parse_object(shader, cur->second);

				BgfxShader bgfxshader(default_allocator());
				if (json_object::has(shader, "includes"))
					sjson::parse_string(bgfxshader._includes, shader["includes"]);
				if (json_object::has(shader, "code"))
					sjson::parse_verbatim(bgfxshader._code, shader["code"]);
				if (json_object::has(shader, "vs_code"))
					sjson::parse_verbatim(bgfxshader._vs_code, shader["vs_code"]);
				if (json_object::has(shader, "fs_code"))
					sjson::parse_verbatim(bgfxshader._fs_code, shader["fs_code"]);
				if (json_object::has(shader, "varying"))
					sjson::parse_verbatim(bgfxshader._varying, shader["varying"]);
				if (json_object::has(shader, "vs_input_output"))
					sjson::parse_verbatim(bgfxshader._vs_input_output, shader["vs_input_output"]);
				if (json_object::has(shader, "fs_input_output"))
					sjson::parse_verbatim(bgfxshader._fs_input_output, shader["fs_input_output"]);
				if (json_object::has(shader, "samplers"))
					parse_bgfx_samplers(bgfxshader, shader["samplers"]);

				DynamicString key(ta);
				key = cur->first;

				DATA_COMPILER_ASSERT(!hash_map::has(_bgfx_shaders, key)
					, _opts
					, "Bgfx shader redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_bgfx_shaders, key, bgfxshader);
			}

			return 0;
		}

		s32 parse_bgfx_samplers(BgfxShader& bgfxshader, const char* json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_samplers(ta);
			sjson::parse_object(bgfx_samplers, json);

			auto cur = json_object::begin(bgfx_samplers);
			auto end = json_object::end(bgfx_samplers);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(bgfx_samplers, cur);

				JsonObject sampler(ta);
				sjson::parse_object(sampler, cur->second);

				DynamicString sampler_state(ta);
				sjson::parse_string(sampler_state, sampler["sampler_state"]);

				DATA_COMPILER_ASSERT(hash_map::has(_sampler_states, sampler_state)
					, _opts
					, "Unknown sampler state: '%s'"
					, sampler_state.c_str()
					);

				DynamicString key(ta);
				key = cur->first;

				DATA_COMPILER_ASSERT(!hash_map::has(bgfxshader._samplers, key)
					, _opts
					, "Bgfx sampler redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(bgfxshader._samplers, key, sampler_state);
			}

			return 0;
		}

		s32 parse_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject shaders(ta);
			sjson::parse_object(shaders, json);

			auto cur = json_object::begin(shaders);
			auto end = json_object::end(shaders);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(shaders, cur);

				JsonObject obj(ta);
				sjson::parse_object(obj, cur->second);

				ShaderPermutation shader(default_allocator());
				sjson::parse_string(shader._bgfx_shader, obj["bgfx_shader"]);
				sjson::parse_string(shader._render_state, obj["render_state"]);

				DynamicString key(ta);
				key = cur->first;

				DATA_COMPILER_ASSERT(!hash_map::has(_shaders, key)
					, _opts
					, "Shader redefined: '%s'"
					, key.c_str()
					);
				hash_map::set(_shaders, key, shader);
			}

			return 0;
		}

		s32 parse_static_compile(const char* json)
		{
			TempAllocator4096 ta;
			JsonArray static_compile(ta);
			sjson::parse_array(static_compile, json);

			for (u32 ii = 0; ii < array::size(static_compile); ++ii)
			{
				JsonObject obj(ta);
				sjson::parse_object(obj, static_compile[ii]);

				StaticCompile sc(default_allocator());
				sjson::parse_string(sc._shader, obj["shader"]);

				JsonArray defines(ta);
				sjson::parse_array(defines, obj["defines"]);
				for (u32 jj = 0; jj < array::size(defines); ++jj)
				{
					DynamicString def(ta);
					sjson::parse_string(def, defines[jj]);
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

			for (u32 ii = 0; ii < vector::size(_static_compile); ++ii)
			{
				const StaticCompile& sc              = _static_compile[ii];
				const DynamicString& shader          = sc._shader;
				const Vector<DynamicString>& defines = sc._defines;

				TempAllocator1024 ta;
				DynamicString str(ta);
				str = shader;
				for (u32 jj = 0; jj < vector::size(defines); ++jj)
				{
					str += "+";
					str += defines[jj];
				}
				const StringId32 shader_name(str.c_str());

				DATA_COMPILER_ASSERT(hash_map::has(_shaders, sc._shader)
					, _opts
					, "Unknown shader: '%s'"
					, shader.c_str()
					);
				const ShaderPermutation sp_default(default_allocator());
				const ShaderPermutation& sp       = hash_map::get(_shaders, shader, sp_default);
				const DynamicString& bgfx_shader  = sp._bgfx_shader;
				const DynamicString& render_state = sp._render_state;

				DATA_COMPILER_ASSERT(hash_map::has(_bgfx_shaders, sp._bgfx_shader)
					, _opts
					, "Unknown bgfx shader: '%s'"
					, bgfx_shader.c_str()
					);
				DATA_COMPILER_ASSERT(hash_map::has(_render_states, sp._render_state)
					, _opts
					, "Unknown render state: '%s'"
					, render_state.c_str()
					);

				const RenderState rs_default;
				const RenderState& rs = hash_map::get(_render_states, render_state, rs_default);

				_opts.write(shader_name._id);                                // Shader name
				_opts.write(rs.encode());                                    // Render state
				compile_sampler_states(bgfx_shader.c_str());                 // Sampler states
				s32 err = compile_bgfx_shader(bgfx_shader.c_str(), defines); // Shader code
				DATA_COMPILER_ENSURE(err == 0, _opts);
			}

			return 0;
		}

		void compile_sampler_states(const char* bgfx_shader)
		{
			TempAllocator512 ta;
			DynamicString key(ta);
			key = bgfx_shader;
			const BgfxShader shader_default(default_allocator());
			const BgfxShader& shader = hash_map::get(_bgfx_shaders, key, shader_default);

			_opts.write(hash_map::size(shader._samplers));

			auto cur = hash_map::begin(shader._samplers);
			auto end = hash_map::end(shader._samplers);
			for (; cur != end; ++cur)
			{
				HASH_MAP_SKIP_HOLE(shader._samplers, cur);

				const DynamicString& name = cur->first;
				const DynamicString& sampler_state = cur->second;
				const SamplerState ss_default;
				const SamplerState& ss = hash_map::get(_sampler_states, sampler_state, ss_default);

				_opts.write(name.to_string_id());
				_opts.write(ss.encode());
			}
		}

		s32 compile_bgfx_shader(const char* bgfx_shader, const Vector<DynamicString>& defines)
		{
			TempAllocator512 taa;
			DynamicString key(taa);
			key = bgfx_shader;
			const BgfxShader shader_default(default_allocator());
			const BgfxShader& shader = hash_map::get(_bgfx_shaders, key, shader_default);

			DynamicString included_code(default_allocator());
			if (!(shader._includes == ""))
			{
				const BgfxShader included_default(default_allocator());
				const BgfxShader& included = hash_map::get(_bgfx_shaders, shader._includes, included_default);
				included_code = included._code;
			}

			StringStream vs_code(default_allocator());
			StringStream fs_code(default_allocator());
			vs_code << shader._vs_input_output.c_str();
			for (u32 i = 0; i < vector::size(defines); ++i)
			{
				vs_code << "#define " << defines[i].c_str() << "\n";
			}
			vs_code << included_code.c_str();
			vs_code << shader._code.c_str();
			vs_code << shader._vs_code.c_str();
			fs_code << shader._fs_input_output.c_str();
			for (u32 i = 0; i < vector::size(defines); ++i)
			{
				fs_code << "#define " << defines[i].c_str() << "\n";
			}
			fs_code << included_code.c_str();
			fs_code << shader._code.c_str();
			fs_code << shader._fs_code.c_str();

			_opts.write_temporary(_vs_src_path.c_str(), vs_code);
			_opts.write_temporary(_fs_src_path.c_str(), fs_code);
			_opts.write_temporary(_varying_path.c_str(), shader._varying.c_str(), shader._varying.length());

			const char* shaderc = _opts.exe_path(shaderc_paths, countof(shaderc_paths));
			DATA_COMPILER_ASSERT(shaderc != NULL, _opts, "shaderc not found");

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
				, _opts.platform()
				);
			if (sc != 0)
			{
				delete_temp_files();
				DATA_COMPILER_ASSERT(sc == 0
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
				, _opts.platform()
				);
			if (sc != 0)
			{
				delete_temp_files();
				DATA_COMPILER_ASSERT(sc == 0
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
			if (ec != 0)
			{
				pr_frag.wait();
				delete_temp_files();
				DATA_COMPILER_ASSERT(false
					, _opts
					, "Failed to compile vertex shader `%s`:\n%s"
					, bgfx_shader
					, string_stream::c_str(output_vert)
					);
			}

			_opts.read_output(output_frag, pr_frag);
			ec = pr_frag.wait();
			if (ec != 0)
			{
				delete_temp_files();
				DATA_COMPILER_ASSERT(false
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
	};

	s32 compile(CompileOptions& opts)
	{
		ShaderCompiler sc(opts);
		s32 err = sc.parse(opts.source_path());
		DATA_COMPILER_ENSURE(err == 0, opts);

		return sc.compile();
	}

} // namespace shader_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
