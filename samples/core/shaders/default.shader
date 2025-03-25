include = ["core/shaders/common.shader"]

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
		includes = "common"

		samplers = {
			u_albedo_map = { sampler_state = "mirror_anisotropic" }
		}

		varying = """
			vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
			vec4 v_view      : TEXCOORD0 = vec4(0.0, 0.0, 0.0, 0.0);
			vec2 v_texcoord0 : TEXCOORD1 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec3 a_normal    : NORMAL;
			vec2 a_texcoord0 : TEXCOORD0;
			vec4 a_indices   : BLENDINDICES;
			vec4 a_weight    : BLENDWEIGHT;
		"""

		vs_input_output = """
		#if defined(SKINNING)
			$input a_position, a_normal, a_texcoord0, a_indices, a_weight
		#else
			$input a_position, a_normal, a_texcoord0
		#endif
			$output v_normal, v_view, v_texcoord0
		"""

		vs_code = """
			void main()
			{
		#if defined(SKINNING)
				mat4 model;
				model  = a_weight.x * u_model[int(a_indices.x)];
				model += a_weight.y * u_model[int(a_indices.y)];
				model += a_weight.z * u_model[int(a_indices.z)];
				model += a_weight.w * u_model[int(a_indices.w)];
				gl_Position = mul(mul(u_modelViewProj, model), vec4(a_position, 1.0));
				v_view = mul(mul(u_modelView, model), vec4(a_position, 1.0));
				v_normal = normalize(mul(mul(u_modelView, model), vec4(a_normal, 0.0)).xyz);
		#else
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_view = mul(u_modelView, vec4(a_position, 1.0));
				v_normal = normalize(mul(u_modelView, vec4(a_normal, 0.0)).xyz);
		#endif

				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_normal, v_view, v_texcoord0
		"""

		fs_code = """
		#if !defined(NO_LIGHT)
			uniform vec4 u_light_position;  // In world-space
			uniform vec4 u_light_direction; // In view-space
			uniform vec4 u_light_color;
			uniform vec4 u_light_range;
			uniform vec4 u_light_intensity;

			uniform vec4 u_ambient;
			uniform vec4 u_diffuse;
			uniform vec4 u_specular;
		#endif

		#if defined(DIFFUSE_MAP)
			SAMPLER2D(u_albedo_map, 0);
		#endif // DIFFUSE_MAP

			void main()
			{
		#if !defined(NO_LIGHT)
				// normalize both input vectors
				vec3 n = normalize(v_normal);
				vec3 e = normalize(v_view.xyz);
				vec3 l = u_light_direction.xyz;

				float nl = max(0.0, dot(n, l));
				vec3 light_diffuse = nl * toLinearAccurate(u_light_color.rgb) * u_light_intensity.x;

				vec3 color = max(u_diffuse.rgb * light_diffuse, u_ambient.rgb);
		#else
				vec3 color = vec3(1.0f, 1.0f, 1.0f);
		#endif // !defined(NO_LIGHT)

		#if defined(DIFFUSE_MAP)
				color = color * texture2D(u_albedo_map, v_texcoord0).rgb;
		#endif

				gl_FragColor = vec4(toGammaAccurate(color), 1.0);
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
	{ shader = "mesh" defines = ["DIFFUSE_MAP" "SKINNING"] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP" "NO_LIGHT"] }
	{ shader = "selection" defines = [] }
	{ shader = "outline" defines = [] }
	{ shader = "blit" defines = [] }
	{ shader = "fallback" defines = [] }
	{ shader = "noop" defines = [] }

]
