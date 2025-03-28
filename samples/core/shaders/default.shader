include = [
	"core/shaders/common.shader"
	"core/shaders/lighting.shader"
]

render_states = {
	debug_line = {
		states = {
			blend_enable = true
			blend_src = "src_alpha"
			blend_dst = "inv_src_alpha"
			primitive_type = "pt_lines"
			"!defined(DEPTH_ENABLED)" = {
				depth_enable = false
			}
		}
	}

	gui = {
		states = {
			cull_mode = "none"
			depth_write_enable = true
			depth_enable = false
			blend_enable = true
			blend_src = "src_alpha"
			blend_dst = "inv_src_alpha"
			"defined(DEPTH_ENABLED)" = {
				depth_enable = true
			}
		}
	}

	blit = {
		states = {
			cull_mode = "none"
			depth_write_enable = false;
			depth_enable = false;
			blend_enable = false;
		}
	}

	sprite = {
		states = {
			depth_func = "always"
			blend_enable = true
			blend_src = "src_alpha"
			blend_dst = "inv_src_alpha"
			blend_equation = "add"
		}
	}

	mesh = {
	}

	selection = {
		states = {
			alpha_write_enable = false
		}
	}

	outline = {
		inherit = "gui"
		states = {
			depth_write_enable = false
		}
	}

	noop = {
		states = {
			rgb_write_enable = false
			alpha_write_enable = false
			depth_write_enable = false
			depth_enable = false
			blend_enable = false
		}
	}
}

bgfx_shaders = {
	debug_line = {
		includes = "common"

		varying = """
			vec4 v_color0   : COLOR0 = vec4(0.0, 0.0, 0.0, 0.0);

			vec3 a_position : POSITION;
			vec4 a_color0   : COLOR0;
		"""

		vs_input_output = """
			$input a_position, a_color0
			$output v_color0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_color0 = a_color0;
			}
		"""

		fs_input_output = """
			$input v_color0
		"""

		fs_code = """
			void main()
			{
				gl_FragColor = v_color0;
			}
		"""
	}

	gui = {
		includes = "common"

		samplers = {
			u_albedo_map = { sampler_state = "clamp_anisotropic" }
		}

		varying = """
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
			vec4 v_color0    : COLOR0 = vec4(0.0, 0.0, 0.0, 0.0);

			vec3 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
			vec4 a_color0    : COLOR0;
		"""

		vs_input_output = """
			$input a_position, a_texcoord0, a_color0
			$output v_texcoord0, v_color0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
		#if defined(DIFFUSE_MAP)
				v_texcoord0 = a_texcoord0;
		#endif // DIFFUSE_MAP
				v_color0 = a_color0;
			}
		"""

		fs_input_output = """
			$input v_texcoord0, v_color0
		"""

		fs_code = """
		#if defined(DIFFUSE_MAP)
			SAMPLER2D(u_albedo_map, 0);
		#endif // DIFFUSE_MAP

			void main()
			{
		#if defined(DIFFUSE_MAP)
				gl_FragColor = toGammaAccurate(texture2D(u_albedo_map, v_texcoord0) * toLinearAccurate(v_color0));
		#else
				gl_FragColor = v_color0;
		#endif // DIFFUSE_MAP
			}
		"""
	}

	sprite = {
		includes = "common"

		samplers = {
			u_albedo_map = { sampler_state = "clamp_point" }
		}

		varying = """
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
		"""

		vs_input_output = """
			$input a_position, a_texcoord0
			$output v_texcoord0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_texcoord0
		"""

		fs_code = """
			SAMPLER2D(u_albedo_map, 0);
			uniform vec4 u_color;

			void main()
			{
				gl_FragColor = toGammaAccurate(texture2D(u_albedo_map, v_texcoord0) * toLinearAccurate(u_color));
			}
		"""
	}

	mesh = {
		includes = [ "common" "lighting" ]

		samplers = {
			u_albedo_map = { sampler_state = "mirror_anisotropic" }
			u_normal_map = { sampler_state = "mirror_anisotropic" }
			u_metallic_map = { sampler_state = "mirror_anisotropic" }
			u_roughness_map = { sampler_state = "mirror_anisotropic" }
			u_ao_map = { sampler_state = "mirror_anisotropic" }
		}

		varying = """
			vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
			vec3 v_tangent   : TANGENT   = vec3(0.0, 0.0, 0.0);
			vec3 v_bitangent : BITANGENT = vec3(0.0, 0.0, 0.0);
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
			vec3 v_position  : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
			vec3 v_camera    : TEXCOORD2 = vec3(0.0, 0.0, 0.0);

			vec3 a_position  : POSITION;
			vec3 a_normal    : NORMAL;
			vec3 a_tangent   : TANGENT;
			vec3 a_bitangent : BITANGENT;
			vec4 a_indices   : BLENDINDICES;
			vec4 a_weight    : BLENDWEIGHT;
			vec2 a_texcoord0 : TEXCOORD0;
		"""

		vs_input_output = """
		#if defined(SKINNING)
			$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0, a_indices, a_weight
		#else
			$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
		#endif
			$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_position, v_camera
		"""

		vs_code = """
			uniform vec4 u_use_normal_map;

			void main()
			{
		#if defined(SKINNING)
				mat4 model;
				model  = a_weight.x * u_model[int(a_indices.x)];
				model += a_weight.y * u_model[int(a_indices.y)];
				model += a_weight.z * u_model[int(a_indices.z)];
				model += a_weight.w * u_model[int(a_indices.w)];
				gl_Position = mul(mul(u_modelViewProj, model), vec4(a_position, 1.0));
				model = mul(u_model[0], model);
		#else
				gl_Position = mul(mul(u_viewProj, u_model[0]), vec4(a_position, 1.0));
				mat4 model = u_model[0];
		#endif

				v_position = mul(model, vec4(a_position, 1.0)).xyz;
				v_normal = normalize(mul(model, vec4(a_normal, 0.0))).xyz;
				v_tangent = normalize(mul(model, vec4(a_tangent, 0.0))).xyz;
				v_bitangent = normalize(mul(model, vec4(a_bitangent, 0.0))).xyz;

				mat3 tbn;
				if (u_use_normal_map.r == 1.0)
					tbn = mtxFromCols(v_tangent, v_bitangent, v_normal);
				else
					tbn = mtxFromCols(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

				v_camera = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
				v_camera = mul(v_camera - v_position, tbn);
				v_position = mul(v_position, tbn);

				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_position, v_camera
		"""

		fs_code = """
			SAMPLER2D(u_albedo_map, 0);
			SAMPLER2D(u_normal_map, 1);
			SAMPLER2D(u_metallic_map, 2);
			SAMPLER2D(u_roughness_map, 3);
			SAMPLER2D(u_ao_map, 4);

			uniform vec4 u_albedo;
			uniform vec4 u_metallic;
			uniform vec4 u_roughness;
			uniform vec4 u_use_albedo_map;
			uniform vec4 u_use_normal_map;
			uniform vec4 u_use_metallic_map;
			uniform vec4 u_use_roughness_map;
			uniform vec4 u_use_ao_map;

			void main()
			{
				vec3 albedo = u_use_albedo_map.r == 1.0 ? texture2D(u_albedo_map, v_texcoord0).rgb : u_albedo.rgb;

		#if defined(NO_LIGHT)
				vec3 radiance = albedo;
		#else
				vec3 normal = u_use_normal_map.r == 1.0 ? decodeNormalUint(texture2D(u_normal_map, v_texcoord0).rgb) : v_normal;
				float metallic = u_use_metallic_map.r == 1.0 ? texture2D(u_metallic_map, v_texcoord0).r : u_metallic.r;
				float roughness = u_use_roughness_map.r == 1.0 ? texture2D(u_roughness_map, v_texcoord0).r: u_roughness.r;
				float ao = u_use_ao_map.r == 1.0 ? texture2D(u_ao_map, v_texcoord0).r : 0.0;

				mat3 tbn;
				if (u_use_normal_map.r == 1.0)
					tbn = mtxFromCols(v_tangent, v_bitangent, v_normal);
				else
					tbn = mtxFromCols(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

				vec3 n = normalize(normal); // Fragment normal.
				vec3 v = normalize(v_camera); // Versor from fragment to camera pos.
				vec3 f0 = mix(vec3_splat(0.04), albedo, metallic);
				vec3 radiance = calc_lighting(tbn, n, v, v_position, albedo, metallic, roughness, f0);
				radiance = radiance / (radiance + vec3_splat(1.0)); // Tone-mapping.
		#endif // !defined(NO_LIGHT)

				gl_FragColor = vec4(toGammaAccurate(radiance), 1.0);
			}
		"""
	}

	selection = {
		includes = "common"

		varying = """
			vec3 a_position  : POSITION;
		"""

		vs_input_output = """
			$input a_position
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			}
		"""

		fs_input_output = """
		"""

		fs_code = """
			uniform vec4 u_unit_id;

			void main()
			{
				gl_FragColor.r = u_unit_id.x;
			}
		"""
	}

	outline = {
		includes = "common"

		varying = """
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
		"""

		vs_input_output = """
			$input a_position, a_texcoord0
			$output v_texcoord0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0) );
				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_texcoord0
		"""

		fs_code = """
		#if !BX_PLATFORM_EMSCRIPTEN
			USAMPLER2D(s_selection, 0);
			SAMPLER2D(s_selection_depth, 1);
			SAMPLER2D(s_main_depth, 2);
			uniform vec4 u_outline_color;

			void main()
			{
				vec2 tex_size = vec2(textureSize(s_selection, 0)) - vec2(1, 1);

				uint id[8];
				id[0] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2(-1, -1)), 0).r;
				id[1] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2( 0, -1)), 0).r;
				id[2] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2( 1, -1)), 0).r;
				id[3] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2( 1,  0)), 0).r;
				id[4] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2( 1,  1)), 0).r;
				id[5] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2( 0,  1)), 0).r;
				id[6] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2(-1,  1)), 0).r;
				id[7] = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size + vec2(-1,  0)), 0).r;

				uint ref_id = texelFetch(s_selection, ivec2(v_texcoord0 * tex_size), 0).r;

				float alpha = 0.0;
				for (int ii = 0; ii < 8; ++ii)
				{
					if (ref_id != id[ii])
						alpha += 1.0/8.0;
				}

				if (alpha == 0.0)
				{
					gl_FragColor = vec4(0, 0, 0, 0);
					return;
				}
				alpha = max(0.5, alpha);

				// Scan the depth around the center and choose the value closest
				// to the viewer. This is to avoid getting s_depth = 1.0.
				float s_depth = 1.0;
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2(-1, -1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 0, -1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 1, -1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 1,  0)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 1,  1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 0,  1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2(-1,  1)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2(-1,  0)), 0).r);
				s_depth = min(s_depth, texelFetch(s_selection_depth, ivec2(v_texcoord0 * tex_size + vec2( 0,  0)), 0).r);

				float m_depth = texelFetch(s_main_depth, ivec2(v_texcoord0 * tex_size), 0).r;

				// Dim alpha if selected object is behind another object.
				if (s_depth > m_depth)
					alpha *= 0.35;

				gl_FragColor = vec4(u_outline_color.xyz, alpha);
			}
		#else
			void main()
			{
				gl_FragColor = vec4_splat(0.0);
			}
		#endif
		"""
	}

	blit = {
		includes = "common"

		varying = """
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
		"""

		vs_input_output = """
			$input a_position, a_texcoord0
			$output v_texcoord0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0) );
				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_texcoord0
		"""

		fs_code = """
			SAMPLER2D(s_color, 0);

			void main()
			{
				gl_FragColor = texture2D(s_color, v_texcoord0);
			}
		"""
	}

	fallback = {
		includes = "common"

		varying = """
			vec3 a_position  : POSITION;
		"""

		vs_input_output = """
			$input a_position
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			}
		"""

		fs_input_output = """
		"""

		fs_code = """
			void main()
			{
				gl_FragColor = toGammaAccurate(vec4(1.0, 0.0, 1.0, 1.0));
			}
		"""
	}

	noop = {
		includes = "common"

		varying = """
		"""

		vs_input_output = """
		"""

		vs_code = """
			void main()
			{
				gl_Position = vec4_splat(0.0);
			}
		"""

		fs_input_output = """
		"""

		fs_code = """
			void main()
			{
				discard;
			}
		"""
	}

}

shaders = {
	debug_line = {
		bgfx_shader = "debug_line"
		render_state = "debug_line"
	}

	gui = {
		bgfx_shader = "gui"
		render_state = "gui"
	}

	sprite = {
		bgfx_shader = "sprite"
		render_state = "sprite"
	}

	mesh = {
		bgfx_shader = "mesh"
		render_state = "mesh"
	}

	selection = {
		bgfx_shader = "selection"
		render_state = "selection"
	}

	outline = {
		bgfx_shader = "outline"
		render_state = "outline"
	}

	blit = {
		bgfx_shader = "blit"
		render_state = "blit"
	}

	fallback = {
		bgfx_shader = "fallback"
		render_state = "mesh"
	}

	noop = {
		bgfx_shader = "noop"
		render_state = "noop"
	}

}

static_compile = [
	{ shader = "debug_line" defines = [] }
	{ shader = "debug_line" defines = ["DEPTH_ENABLED"] }
	{ shader = "gui" defines = [] }
	{ shader = "gui" defines = ["DIFFUSE_MAP"]}
	{ shader = "gui" defines = ["DEPTH_ENABLED"]}
	{ shader = "gui" defines = ["DIFFUSE_MAP" "DEPTH_ENABLED"]}
	{ shader = "sprite" defines = [] }
	{ shader = "mesh" defines = [] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP"] }
	{ shader = "mesh" defines = ["SKINNING"] }
	{ shader = "mesh" defines = ["NO_LIGHT"] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP" "SKINNING"] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP" "NO_LIGHT"] }
	{ shader = "selection" defines = [] }
	{ shader = "outline" defines = [] }
	{ shader = "blit" defines = [] }
	{ shader = "fallback" defines = [] }
	{ shader = "noop" defines = [] }

]
