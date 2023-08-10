include = ["core/shaders/common.shader"]

render_states = {
	debug_line = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = true
		depth_enable = true
		depth_func = "lequal"
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		primitive_type = "pt_lines"
		cull_mode = "cw"
	}

	debug_line_noz = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = true
		depth_enable = false
		depth_func = "lequal"
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		primitive_type = "pt_lines"
		cull_mode = "cw"
	}

	gui = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = false
		depth_enable = false
		depth_func = "lequal"
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		cull_mode = "cw"
	}

	gui_noblend = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = false
		depth_enable = false
		depth_func = "lequal"
		blend_enable = false
		cull_mode = "cw"
	}

	sprite = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = true
		depth_enable = true
		depth_func = "always"
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		blend_equation = "add"
		cull_mode = "cw"
	}

	mesh = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_write_enable = true
		depth_enable = true
		depth_func = "lequal"
		blend_enable = false
		cull_mode = "cw"
	}

	selection = {
		rgb_write_enable = true
		alpha_write_enable = false
		depth_write_enable = true
		depth_enable = true
		depth_func = "lequal"
		blend_enable = false
		cull_mode = "cw"
	}

	noop = {
		rgb_write_enable = false
		alpha_write_enable = false
		depth_write_enable = false
		depth_enable = false
		blend_enable = false
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
				v_color0 = toLinearAccurate(a_color0);
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
			u_albedo = { sampler_state = "clamp_anisotropic" }
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
		#ifdef DIFFUSE_MAP
				v_texcoord0 = a_texcoord0;
		#endif // DIFFUSE_MAP
				v_color0 = toLinearAccurate(a_color0);
			}
		"""

		fs_input_output = """
			$input v_texcoord0, v_color0
		"""

		fs_code = """
		#ifdef DIFFUSE_MAP
			SAMPLER2D(u_albedo, 0);
		#endif // DIFFUSE_MAP

			void main()
			{
		#ifdef DIFFUSE_MAP
				gl_FragColor = texture2D(u_albedo, v_texcoord0) * v_color0;
		#else
				gl_FragColor = v_color0;
		#endif // DIFFUSE_MAP
			}
		"""
	}

	sprite = {
		includes = "common"

		samplers = {
			u_albedo = { sampler_state = "clamp_point" }
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
			uniform vec4 u_color;
			SAMPLER2D(u_albedo, 0);

			void main()
			{
				gl_FragColor = texture2D(u_albedo, v_texcoord0) * toLinearAccurate(u_color);
			}
		"""
	}

	mesh = {
		includes = "common"

		samplers = {
			u_albedo = { sampler_state = "mirror_anisotropic" }
		}

		varying = """
			vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
			vec4 v_view      : TEXCOORD0 = vec4(0.0, 0.0, 0.0, 0.0);
			vec2 v_texcoord0 : TEXCOORD1 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec3 a_normal    : NORMAL;
			vec2 a_texcoord0 : TEXCOORD0;
		"""

		vs_input_output = """
			$input a_position, a_normal, a_texcoord0
			$output v_normal, v_view, v_texcoord0
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_view = mul(u_modelView, vec4(a_position, 1.0));
				v_normal = normalize(mul(u_modelView, vec4(a_normal, 0.0)).xyz);

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

		#ifdef DIFFUSE_MAP
			SAMPLER2D(u_albedo, 0);
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

		#ifdef DIFFUSE_MAP
				gl_FragColor.rgb = color * texture2D(u_albedo, v_texcoord0).rgb;
		#else
				gl_FragColor.rgb = color;
		#endif // DIFFUSE_MAP
				gl_FragColor.a = 1.0;
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
				gl_FragColor = toGammaAccurate(texture2D(s_color, v_texcoord0));
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
				gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
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

	debug_line_noz = {
		bgfx_shader = "debug_line"
		render_state = "debug_line_noz"
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
		render_state = "gui"
	}

	blit = {
		bgfx_shader = "blit"
		render_state = "gui_noblend"
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
	{ shader = "debug_line_noz" defines = [] }
	{ shader = "gui" defines = [] }
	{ shader = "gui" defines = ["DIFFUSE_MAP"]}
	{ shader = "sprite" defines = [] }
	{ shader = "mesh" defines = [] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP"] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP" "NO_LIGHT"] }
	{ shader = "selection" defines = [] }
	{ shader = "outline" defines = [] }
	{ shader = "blit" defines = [] }
	{ shader = "fallback" defines = [] }
	{ shader = "noop" defines = [] }

]
