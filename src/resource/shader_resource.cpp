/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "compile_options.h"
#include "config.h"
#include "device.h"
#include "filesystem.h"
#include "map.h"
#include "os.h"
#include "resource_manager.h"
#include "shader_manager.h"
#include "shader_resource.h"
#include "sjson.h"
#include "string_stream.h"
#include "temp_allocator.h"

#if CROWN_DEBUG
#	define SHADERC_NAME "shaderc-debug-"
#else
#	define SHADERC_NAME "shaderc-development-"
#endif // CROWN_DEBUG
#if CROWN_ARCH_32BIT
#	define SHADERC_BITS "32"
#elif CROWN_ARCH_64BIT
#	define SHADERC_BITS "64"
#endif // CROWN_ARCH_32BIT
#if CROWN_PLATFORM_LINUX
#	define SHADERC_PATH "./" SHADERC_NAME "" SHADERC_BITS
#elif CROWN_PLATFORM_WINDOWS
#	define SHADERC_PATH SHADERC_NAME "" SHADERC_BITS ".exe"
#else
# 	define SHADERC_PATH ""
#endif // CROWN_PLATFORM_LINUX

namespace crown
{
namespace shader_resource
{
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

			COUNT
		};
	};

	struct PrimitiveType
	{
		enum Enum
		{
			PT_TRISTRIP,
			PT_LINES,
			PT_LINESTRIP,
			PT_POINTS,

			COUNT
		};
	};

	struct DepthTestInfo
	{
		const char* name;
		DepthFunction::Enum value;
	};

	static DepthTestInfo _depth_test_map[] =
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
	CE_STATIC_ASSERT(CE_COUNTOF(_depth_test_map) == DepthFunction::COUNT);

	struct BlendFunctionInfo
	{
		const char* name;
		BlendFunction::Enum value;
	};

	static BlendFunctionInfo _blend_function_map[] =
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
	CE_STATIC_ASSERT(CE_COUNTOF(_blend_function_map) == BlendFunction::COUNT);

	struct BlendEquationInfo
	{
		const char* name;
		BlendEquation::Enum value;
	};

	static BlendEquationInfo _blend_equation_map[] =
	{
		{ "add",    BlendEquation::ADD    },
		{ "sub",    BlendEquation::SUB    },
		{ "revsub", BlendEquation::REVSUB },
		{ "min",    BlendEquation::MIN    },
		{ "max",    BlendEquation::MAX    }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_blend_equation_map) == BlendEquation::COUNT);

	struct CullModeInfo
	{
		const char* name;
		CullMode::Enum value;
	};

	static CullModeInfo _cull_mode_map[] =
	{
		{ "cw",  CullMode::CW  },
		{ "ccw", CullMode::CCW }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_cull_mode_map) == CullMode::COUNT);

	struct PrimitiveTypeInfo
	{
		const char* name;
		PrimitiveType::Enum value;
	};

	static PrimitiveTypeInfo _primitive_type_map[] =
	{
		{ "pt_tristrip",  PrimitiveType::PT_TRISTRIP  },
		{ "pt_lines",     PrimitiveType::PT_LINES     },
		{ "pt_linestrip", PrimitiveType::PT_LINESTRIP },
		{ "pt_points",    PrimitiveType::PT_POINTS    }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_primitive_type_map) == PrimitiveType::COUNT);

	static uint64_t _bgfx_depth_function_map[] =
	{
		BGFX_STATE_DEPTH_TEST_LESS,     // DepthFunction::LESS
		BGFX_STATE_DEPTH_TEST_LEQUAL,   // DepthFunction::LEQUAL
		BGFX_STATE_DEPTH_TEST_EQUAL,    // DepthFunction::EQUAL
		BGFX_STATE_DEPTH_TEST_GEQUAL,   // DepthFunction::GEQUAL
		BGFX_STATE_DEPTH_TEST_GREATER,  // DepthFunction::GREATER
		BGFX_STATE_DEPTH_TEST_NOTEQUAL, // DepthFunction::NOTEQUAL
		BGFX_STATE_DEPTH_TEST_NEVER,    // DepthFunction::NEVER
		BGFX_STATE_DEPTH_TEST_ALWAYS,   // DepthFunction::ALWAYS
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_bgfx_depth_function_map) == DepthFunction::COUNT);

	static uint64_t _bgfx_blend_function_map[] =
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
		BGFX_STATE_BLEND_INV_FACTOR,    // BlendFunction::INV_FACTOR
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_bgfx_blend_function_map) == BlendFunction::COUNT);

	static uint64_t _bgfx_blend_equation_map[] =
	{
		BGFX_STATE_BLEND_EQUATION_ADD,    // BlendEquation::ADD
		BGFX_STATE_BLEND_EQUATION_SUB,    // BlendEquation::SUB
		BGFX_STATE_BLEND_EQUATION_REVSUB, // BlendEquation::REVSUB
		BGFX_STATE_BLEND_EQUATION_MIN,    // BlendEquation::MIN
		BGFX_STATE_BLEND_EQUATION_MAX,    // BlendEquation::MAX
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_bgfx_blend_equation_map) == BlendEquation::COUNT);

	static uint64_t _bgfx_cull_mode_map[] =
	{
		BGFX_STATE_CULL_CW,  // CullMode::CW
		BGFX_STATE_CULL_CCW, // CullMode::CCW
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_bgfx_cull_mode_map) == CullMode::COUNT);

	static uint64_t _bgfx_primitive_type_map[] =
	{
		BGFX_STATE_PT_TRISTRIP,  // PrimitiveType::PT_TRISTRIP
		BGFX_STATE_PT_LINES,     // PrimitiveType::PT_LINES
		BGFX_STATE_PT_LINESTRIP, // PrimitiveType::PT_LINESTRIP
		BGFX_STATE_PT_POINTS,    // PrimitiveType::PT_POINTS
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_bgfx_primitive_type_map) == PrimitiveType::COUNT);

	static DepthFunction::Enum name_to_depth_function(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_depth_test_map); ++i)
		{
			if (strcmp(name, _depth_test_map[i].name) == 0)
				return _depth_test_map[i].value;
		}

		return DepthFunction::COUNT;
	}

	static BlendFunction::Enum name_to_blend_function(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_blend_function_map); ++i)
		{
			if (strcmp(name, _blend_function_map[i].name) == 0)
				return _blend_function_map[i].value;
		}

		return BlendFunction::COUNT;
	}

	static BlendEquation::Enum name_to_blend_equation(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_blend_equation_map); ++i)
		{
			if (strcmp(name, _blend_equation_map[i].name) == 0)
				return _blend_equation_map[i].value;
		}

		return BlendEquation::COUNT;
	}

	static CullMode::Enum name_to_cull_mode(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_cull_mode_map); ++i)
		{
			if (strcmp(name, _cull_mode_map[i].name) == 0)
				return _cull_mode_map[i].value;
		}

		return CullMode::COUNT;
	}

	static PrimitiveType::Enum name_to_primitive_type(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_primitive_type_map); ++i)
		{
			if (strcmp(name, _primitive_type_map[i].name) == 0)
				return _primitive_type_map[i].value;
		}

		return PrimitiveType::COUNT;
	}

	static int run_external_compiler(const char* infile, const char* outfile, const char* varying, const char* type, const char* platform, StringStream& output)
	{
		using namespace string_stream;

		TempAllocator512 ta;
		StringStream args(ta);
		args << " -f " << infile;
		args << " -o " << outfile;
		args << " --varyingdef " << varying;
		args << " --type " << type;
		args << " --platform " << platform;
		args << " --profile ";
#if CROWN_PLATFORM_LINUX
		args <<	"120";
#elif CROWN_PLATFORM_WINDOWS
		args << ((strcmp(type, "vertex") == 0) ? "vs_3_0" : "ps_3_0");
#endif

		return os::execute_process(SHADERC_PATH, c_str(args), output);
	}

	struct RenderState
	{
		bool _rgb_write_enable;
		bool _alpha_write_enable;
		bool _depth_write_enable;
		bool _depth_test_enable;
		bool _blend_enable;
		DepthFunction::Enum _depth_function;
		BlendFunction::Enum _blend_src;
		BlendFunction::Enum _blend_dst;
		BlendEquation::Enum _blend_equation;
		CullMode::Enum _cull_mode;
		PrimitiveType::Enum _primitive_type;

		void reset()
		{
			_rgb_write_enable = false;
			_alpha_write_enable = false;
			_depth_write_enable = false;
			_depth_test_enable = false;
			_blend_enable = false;
			_depth_function = DepthFunction::COUNT;
			_blend_src = BlendFunction::COUNT;
			_blend_dst = BlendFunction::COUNT;
			_blend_equation = BlendEquation::COUNT;
			_cull_mode = CullMode::COUNT;
			_primitive_type = PrimitiveType::COUNT;
		}

		uint64_t encode() const
		{
			const uint64_t depth_func = (_depth_test_enable
				? _bgfx_depth_function_map[_depth_function]
				: 0
				);
			const uint64_t blend_func = (_blend_enable
				? BGFX_STATE_BLEND_FUNC(_bgfx_blend_function_map[_blend_src], _bgfx_blend_function_map[_blend_dst])
				: 0
				);
			const uint64_t blend_eq = (_blend_enable
				? BGFX_STATE_BLEND_EQUATION(_bgfx_blend_equation_map[_blend_equation])
				: 0
				);
			const uint64_t cull_mode = (_cull_mode != CullMode::COUNT
				? _bgfx_cull_mode_map[_cull_mode]
				: 0
				);
			const uint64_t primitive_type = (_primitive_type != PrimitiveType::COUNT
				? _bgfx_primitive_type_map[_primitive_type]
				: 0
				);

			uint64_t state = 0;
			state |= (_rgb_write_enable   ? BGFX_STATE_RGB_WRITE   : 0);
			state |= (_alpha_write_enable ? BGFX_STATE_ALPHA_WRITE : 0);
			state |= (_depth_write_enable ? BGFX_STATE_DEPTH_WRITE : 0);
			state |= depth_func;
			state |= blend_func;
			state |= blend_eq;
			state |= cull_mode;
			state |= primitive_type;

			return state;
		}
	};

	struct BgfxShader
	{
		BgfxShader()
			: _includes(default_allocator())
			, _code(default_allocator())
			, _vs_code(default_allocator())
			, _fs_code(default_allocator())
			, _varying(default_allocator())
			, _vs_input_output(default_allocator())
			, _fs_input_output(default_allocator())
		{
		}

		BgfxShader(Allocator& a)
			: _includes(a)
			, _code(a)
			, _vs_code(a)
			, _fs_code(a)
			, _varying(a)
			, _vs_input_output(a)
			, _fs_input_output(a)
		{
		}

		DynamicString _includes;
		DynamicString _code;
		DynamicString _vs_code;
		DynamicString _fs_code;
		DynamicString _varying;
		DynamicString _vs_input_output;
		DynamicString _fs_input_output;

		ALLOCATOR_AWARE;
	};

	struct ShaderPermutation
	{
		ShaderPermutation()
			: _bgfx_shader(default_allocator())
			, _render_state(default_allocator())
		{
		}

		ShaderPermutation(Allocator& a)
			: _bgfx_shader(a)
			, _render_state(a)
		{
		}

		DynamicString _bgfx_shader;
		DynamicString _render_state;
	};

	struct ShaderCompiler
	{
		CompileOptions& _opts;
		Map<DynamicString, RenderState> _render_states;
		Map<DynamicString, BgfxShader> _bgfx_shaders;
		Map<DynamicString, ShaderPermutation> _shaders;

		DynamicString _vs_source_path;
		DynamicString _fs_source_path;
		DynamicString _varying_path;
		DynamicString _vs_compiled_path;
		DynamicString _fs_compiled_path;

		ShaderCompiler(CompileOptions& opts)
			: _opts(opts)
			, _render_states(default_allocator())
			, _bgfx_shaders(default_allocator())
			, _shaders(default_allocator())
			, _vs_source_path(default_allocator())
			, _fs_source_path(default_allocator())
			, _varying_path(default_allocator())
			, _vs_compiled_path(default_allocator())
			, _fs_compiled_path(default_allocator())
		{
			_opts.get_absolute_path("vs_source.sc.temp", _vs_source_path);
			_opts.get_absolute_path("fs_source.sc.temp", _fs_source_path);
			_opts.get_absolute_path("varying.sc.temp", _varying_path);
			_opts.get_absolute_path("vs_compiled.bin.temp", _vs_compiled_path);
			_opts.get_absolute_path("fs_compiled.bin.temp", _fs_compiled_path);
		}

		void parse(const char* path)
		{
			parse(_opts.read(path));
		}

		void parse(Buffer b)
		{
			TempAllocator4096 ta;
			JsonObject object(ta);
			sjson::parse(b, object);

			if (map::has(object, FixedString("include")))
			{
				JsonArray arr(ta);
				sjson::parse_array(object["include"], arr);

				for (uint32_t i = 0; i < array::size(arr); ++i)
				{
					DynamicString path(ta);
					sjson::parse_string(arr[i], path);
					parse(path.c_str());
				}
			}

			if (map::has(object, FixedString("render_states")))
				parse_render_states(object["render_states"]);

			if (map::has(object, FixedString("bgfx_shaders")))
				parse_bgfx_shaders(object["bgfx_shaders"]);

			if (map::has(object, FixedString("shaders")))
				parse_shaders(object["shaders"]);
		}

		void parse_render_states(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject render_states(ta);
			sjson::parse_object(json, render_states);

			auto begin = map::begin(render_states);
			auto end = map::end(render_states);
			for (; begin != end; ++begin)
			{
				JsonObject obj(ta);
				sjson::parse_object(begin->pair.second, obj);

				const bool rgb_write_enable   = sjson::parse_bool(obj["rgb_write_enable"]);
				const bool alpha_write_enable = sjson::parse_bool(obj["alpha_write_enable"]);
				const bool depth_write_enable = sjson::parse_bool(obj["depth_write_enable"]);
				const bool depth_test_enable  = sjson::parse_bool(obj["depth_test_enable"]);
				const bool blend_enable       = sjson::parse_bool(obj["blend_enable"]);

				const bool has_depth_function = map::has(obj, FixedString("depth_function"));
				const bool has_blend_src      = map::has(obj, FixedString("blend_src"));
				const bool has_blend_dst      = map::has(obj, FixedString("blend_dst"));
				const bool has_blend_equation = map::has(obj, FixedString("blend_equation"));
				const bool has_cull_mode      = map::has(obj, FixedString("cull_mode"));
				const bool has_primitive_type = map::has(obj, FixedString("primitive_type"));

				DepthFunction::Enum df    = DepthFunction::COUNT;
				BlendFunction::Enum bfsrc = BlendFunction::COUNT;
				BlendFunction::Enum bfdst = BlendFunction::COUNT;
				BlendEquation::Enum be    = BlendEquation::COUNT;
				CullMode::Enum cm         = CullMode::COUNT;
				PrimitiveType::Enum pt    = PrimitiveType::COUNT;

				DynamicString depth_function(ta);
				DynamicString blend_src(ta);
				DynamicString blend_dst(ta);
				DynamicString blend_equation(ta);
				DynamicString cull_mode(ta);
				DynamicString primitive_type(ta);

				if (has_depth_function)
				{
					sjson::parse_string(obj["depth_function"], depth_function);
					df = name_to_depth_function(depth_function.c_str());
					RESOURCE_COMPILER_ASSERT(df != DepthFunction::COUNT
						, _opts
						, "Unknown depth test: '%s"
						, depth_function.c_str()
						);
				}

				if (has_blend_src)
				{
					sjson::parse_string(obj["blend_src"], blend_src);
					bfsrc = name_to_blend_function(blend_src.c_str());
					RESOURCE_COMPILER_ASSERT(bfsrc != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s"
						, blend_src.c_str()
						);
				}

				if (has_blend_dst)
				{
					sjson::parse_string(obj["blend_dst"], blend_dst);
					bfdst = name_to_blend_function(blend_dst.c_str());
					RESOURCE_COMPILER_ASSERT(bfdst != BlendFunction::COUNT
						, _opts
						, "Unknown blend function: '%s"
						, blend_dst.c_str()
						);
				}

				if (has_blend_equation)
				{
					sjson::parse_string(obj["blend_equation"], blend_equation);
					be = name_to_blend_equation(blend_equation.c_str());
					RESOURCE_COMPILER_ASSERT(be != BlendEquation::COUNT
						, _opts
						, "Unknown blend equation: '%s"
						, blend_equation.c_str()
						);
				}

				if (has_cull_mode)
				{
					sjson::parse_string(obj["cull_mode"], cull_mode);
					cm = name_to_cull_mode(cull_mode.c_str());
					RESOURCE_COMPILER_ASSERT(cm != CullMode::COUNT
						, _opts
						, "Unknown cull mode: '%s"
						, cull_mode.c_str()
						);
				}

				if (has_primitive_type)
				{
					sjson::parse_string(obj["primitive_type"], primitive_type);
					pt = name_to_primitive_type(primitive_type.c_str());
					RESOURCE_COMPILER_ASSERT(pt != PrimitiveType::COUNT
						, _opts
						, "Unknown primitive type: '%s"
						, primitive_type.c_str()
						);
				}

				RenderState rs;
				rs.reset();
				rs._rgb_write_enable   = rgb_write_enable;
				rs._alpha_write_enable = alpha_write_enable;
				rs._depth_write_enable = depth_write_enable;
				rs._depth_test_enable  = depth_test_enable;
				rs._blend_enable       = blend_enable;
				rs._depth_function     = df;
				rs._blend_src          = bfsrc;
				rs._blend_dst          = bfdst;
				rs._blend_equation     = be;
				rs._cull_mode          = cm;
				rs._primitive_type     = pt;

				DynamicString key(ta);
				key = begin->pair.first;

				map::set(_render_states, key, rs);
			}
		}

		void parse_bgfx_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_shaders(ta);
			sjson::parse_object(json, bgfx_shaders);

			auto begin = map::begin(bgfx_shaders);
			auto end = map::end(bgfx_shaders);
			for (; begin != end; ++begin)
			{
				JsonObject shader(ta);
				sjson::parse_object(begin->pair.second, shader);

				BgfxShader bgfxshader(default_allocator());
				if (map::has(shader, FixedString("includes")))
					sjson::parse_string(shader["includes"], bgfxshader._includes);
				if (map::has(shader, FixedString("code")))
					sjson::parse_string(shader["code"], bgfxshader._code);
				if (map::has(shader, FixedString("vs_code")))
					sjson::parse_string(shader["vs_code"], bgfxshader._vs_code);
				if (map::has(shader, FixedString("fs_code")))
					sjson::parse_string(shader["fs_code"], bgfxshader._fs_code);
				if (map::has(shader, FixedString("varying")))
					sjson::parse_string(shader["varying"], bgfxshader._varying);
				if (map::has(shader, FixedString("vs_input_output")))
					sjson::parse_string(shader["vs_input_output"], bgfxshader._vs_input_output);
				if (map::has(shader, FixedString("fs_input_output")))
					sjson::parse_string(shader["fs_input_output"], bgfxshader._fs_input_output);

				DynamicString key(ta);
				key = begin->pair.first;

				map::set(_bgfx_shaders, key, bgfxshader);
			}
		}

		void parse_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject shaders(ta);
			sjson::parse_object(json, shaders);

			auto begin = map::begin(shaders);
			auto end = map::end(shaders);
			for (; begin != end; ++begin)
			{
				JsonObject obj(ta);
				sjson::parse_object(begin->pair.second, obj);

				ShaderPermutation shader(default_allocator());
				sjson::parse_string(obj["bgfx_shader"], shader._bgfx_shader);
				sjson::parse_string(obj["render_state"], shader._render_state);

				DynamicString key(ta);
				key = begin->pair.first;

				map::set(_shaders, key, shader);
			}
		}

		void delete_temp_files()
		{
			const char* vs_source_path   = _vs_source_path.c_str();
			const char* fs_source_path   = _fs_source_path.c_str();
			const char* varying_path     = _varying_path.c_str();
			const char* vs_compiled_path = _vs_compiled_path.c_str();
			const char* fs_compiled_path = _fs_compiled_path.c_str();

			if (_opts.file_exists(vs_source_path))
				_opts.delete_file(vs_source_path);

			if (_opts.file_exists(fs_source_path))
				_opts.delete_file(fs_source_path);

			if (_opts.file_exists(varying_path))
				_opts.delete_file(varying_path);

			if (_opts.file_exists(vs_compiled_path))
				_opts.delete_file(vs_compiled_path);

			if (_opts.file_exists(fs_compiled_path))
				_opts.delete_file(fs_compiled_path);
		}

		void compile()
		{
			_opts.write(SHADER_VERSION);
			_opts.write(map::size(_shaders));

			auto begin = map::begin(_shaders);
			auto end = map::end(_shaders);
			for (; begin != end; ++begin)
			{
				const ShaderPermutation& sp = begin->pair.second;
				const StringId32 shader_name = begin->pair.first.to_string_id();
				const char* bgfx_shader = sp._bgfx_shader.c_str();
				const char* render_state = sp._render_state.c_str();

				RESOURCE_COMPILER_ASSERT(map::has(_bgfx_shaders, sp._bgfx_shader)
					, _opts
					, "Unknown bgfx shader: '%s'"
					, bgfx_shader
					);
				RESOURCE_COMPILER_ASSERT(map::has(_render_states, sp._render_state)
					, _opts
					, "Unknown render state: '%s'"
					, render_state
					);

				const RenderState& rs = _render_states[render_state];

				_opts.write(shader_name._id); // Shader name
				_opts.write(rs.encode());     // Render state
				compile(bgfx_shader);         // Shader code
			}
		}

		void compile(const char* bgfx_shader)
		{
			const BgfxShader& shader = _bgfx_shaders[bgfx_shader];

			DynamicString included_code(default_allocator());
			if (!(shader._includes == ""))
			{
				const BgfxShader& included = _bgfx_shaders[shader._includes.c_str()];
				included_code = included._code;
			}

			using namespace string_stream;

			StringStream vs_code(default_allocator());
			StringStream fs_code(default_allocator());
			vs_code << shader._vs_input_output.c_str();
			vs_code << included_code.c_str();
			vs_code << shader._code.c_str();
			vs_code << shader._vs_code.c_str();
			fs_code << shader._fs_input_output.c_str();
			fs_code << included_code.c_str();
			fs_code << shader._code.c_str();
			fs_code << shader._fs_code.c_str();

			File* vs_file = _opts._fs.open(_vs_source_path.c_str(), FileOpenMode::WRITE);
			vs_file->write(c_str(vs_code), array::size(vs_code));
			_opts._fs.close(*vs_file);

			File* fs_file = _opts._fs.open(_fs_source_path.c_str(), FileOpenMode::WRITE);
			fs_file->write(c_str(fs_code), array::size(fs_code));
			_opts._fs.close(*fs_file);

			File* varying_file = _opts._fs.open(_varying_path.c_str(), FileOpenMode::WRITE);
			varying_file->write(shader._varying.c_str(), shader._varying.length());
			_opts._fs.close(*varying_file);

			TempAllocator4096 ta;
			StringStream output(ta);

			int exitcode = run_external_compiler(_vs_source_path.c_str()
				, _vs_compiled_path.c_str()
				, _varying_path.c_str()
				, "vertex"
				, _opts.platform()
				, output
				);
			if (exitcode)
			{
				delete_temp_files();
				RESOURCE_COMPILER_ASSERT(false
					, _opts
					, "Failed to compile vertex shader:\n%s"
					, c_str(output)
					);
			}

			array::clear(output);
			exitcode = run_external_compiler(_fs_source_path.c_str()
				, _fs_compiled_path.c_str()
				, _varying_path.c_str()
				, "fragment"
				, _opts.platform()
				, output
				);
			if (exitcode)
			{
				delete_temp_files();
				RESOURCE_COMPILER_ASSERT(false
					, _opts
					, "Failed to compile fragment shader:\n%s"
					, c_str(output)
					);
			}

			Buffer tmpvs = _opts.read(_vs_compiled_path.c_str());
			Buffer tmpfs = _opts.read(_fs_compiled_path.c_str());

			delete_temp_files();

			// Write
			_opts.write(uint32_t(array::size(tmpvs)));
			_opts.write(array::begin(tmpvs), array::size(tmpvs));
			_opts.write(uint32_t(array::size(tmpfs)));
			_opts.write(array::begin(tmpfs), array::size(tmpfs));
		}
	};

	void compile(const char* path, CompileOptions& opts)
	{
		ShaderCompiler sc(opts);
		sc.parse(path);
		sc.compile();
	}

	void* load(File& file, Allocator& a)
	{
		return device()->shader_manager()->load(file, a);
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		device()->shader_manager()->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		device()->shader_manager()->offline(id, rm);
	}

	void unload(Allocator& a, void* res)
	{
		device()->shader_manager()->unload(a, res);
	}
} // namespace shader_resource
} // namespace crown
