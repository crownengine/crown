include = ["core/shaders/common.shader"]

render_states = {
	debug_line = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_func = "lequal"
		depth_enable = true
		depth_write_enable = true
		blend_enable = false
		primitive_type = "pt_lines"
		cull_mode = "cw"
	}

	debug_line_noz = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_func = "lequal"
		depth_enable = false
		depth_write_enable = true
		blend_enable = false
		primitive_type = "pt_lines"
		cull_mode = "cw"
	}

	gui = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_func = "lequal"
		depth_enable = false
		depth_write_enable = false
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		cull_mode = "cw"
	}

	sprite = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_func = "lequal"
		depth_enable = true
		depth_write_enable = true
		blend_enable = true
		blend_src = "src_alpha"
		blend_dst = "inv_src_alpha"
		cull_mode = "cw"
	}

	mesh = {
		rgb_write_enable = true
		alpha_write_enable = true
		depth_func = "lequal"
		depth_enable = true
		depth_write_enable = true
		blend_enable = false
		cull_mode = "ccw"
	}
}

bgfx_shaders = {
	debug_line = {
		includes = "common"

		varying = "
			vec4 v_color0   : COLOR0 = vec4(1.0, 0.0, 0.0, 1.0);

			vec3 a_position : POSITION;
			vec4 a_color0   : COLOR0;
		"

		vs_input_output = "
			$input a_position, a_color0
			$output v_color0
		"

		vs_code = "
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_color0 = a_color0;
			}
		"

		fs_input_output = "
			$input v_color0
		"

		fs_code = "
			void main()
			{
				gl_FragColor = v_color0;
			}
		"
	}

	gui = {
		includes = "common"

		varying = "
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
			vec4 v_color0    : COLOR0 = vec4(0.0, 0.0, 0.0, 0.0);

			vec3 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
			vec4 a_color0    : COLOR0;
		"

		vs_input_output = "
			$input = a_position, a_texcoord0, a_color0
			$output v_texcoord0, v_color0
		"

		vs_code = "
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			#ifdef DIFFUSE_MAP
				v_texcoord0 = a_texcoord0;
			#endif // DIFFUSE_MAP
				v_color0 = a_color0;
			}
		"

		fs_input_output = "
			$input v_texcoord0, v_color0
		"

		fs_code = "
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
		"
	}

	sprite = {
		includes = "common"

		varying = "
			vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

			vec2 a_position  : POSITION;
			vec2 a_texcoord0 : TEXCOORD0;
		"

		vs_input_output = "
			$input = a_position, a_texcoord0
			$output v_texcoord0
		"

		vs_code = "
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0));
				v_texcoord0 = a_texcoord0;
			}
		"

		fs_input_output = "
			$input v_texcoord0
		"

		fs_code = "
			SAMPLER2D(u_albedo, 0);
			uniform vec3 u_color = vec3(1, 1, 1);

			void main()
			{
				gl_FragColor = texture2D(u_albedo, v_texcoord0) * vec4(u_color, 1.0);
			}
		"
	}

	mesh = {
		includes = "common"

		varying =
		"
			vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
			vec4 v_view      : TEXCOORD0 = vec4(0.0, 0.0, 0.0, 0.0);
			vec2 v_uv0       : TEXCOORD1 = vec2(0.0, 0.0);

			vec3 a_position  : POSITION;
			vec3 a_normal    : NORMAL;
			vec2 a_texcoord0 : TEXCOORD0;
		"

		vs_input_output =
		"
			$input a_position, a_normal, a_texcoord0
			$output v_normal, v_view, v_uv0
		"

		vs_code =
		"
			void main()
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				v_view = mul(u_modelView, vec4(a_position, 1.0));
				v_normal = normalize(mul(u_modelView, vec4(a_normal, 0.0)).xyz);

				v_uv0 = a_texcoord0;
			}
		"

		fs_input_output =
		"
			$input v_normal, v_view, v_uv0
		"

		fs_code =
		"
			#ifdef DIFFUSE_MAP
			SAMPLER2D(u_albedo, 0);
			#endif // DIFFUSE_MAP

			uniform vec4 u_light_pos; // In world-space
			uniform vec4 u_light_dir; // In view-space
			uniform vec4 u_light_col;

			uniform vec4 u_ambient  = vec4(0.5f, 0.5f, 0.5f, 0.0f);
			uniform vec4 u_diffuse  = vec4(1.0f, 1.0f, 1.0f, 0.0f);
			uniform vec4 u_specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);

			void main()
			{
				// normalize both input vectors
				vec3 n = normalize(v_normal);
				vec3 e = normalize(v_view.xyz);

				float intensity = max(0.0f, dot(n, u_light_dir.xyz));

				vec4 colorOut = max(intensity * (u_diffuse * u_light_col), u_ambient);
			#ifdef DIFFUSE_MAP
				gl_FragColor = colorOut * texture2D(u_albedo, v_uv0);
			#else
				gl_FragColor = colorOut;
			#endif // DIFFUSE_MAP
			}
		"
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
}

static_compile = [
	{ shader = "debug_line" defines = [] }
	{ shader = "debug_line_noz" defines = [] }
	{ shader = "gui" defines = [] }
	{ shader = "gui" defines = ["DIFFUSE_MAP"]}
	{ shader = "sprite" defines = [] }
	{ shader = "mesh" defines = [] }
	{ shader = "mesh" defines = ["DIFFUSE_MAP"] }
]
