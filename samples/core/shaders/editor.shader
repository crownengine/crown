include = [
	"core/shaders/common.shader"
	"core/shaders/default.shader"
]

render_states = {
	selection = {
		inherit = "default"
		states = {
			alpha_write_enable = false
		}
	}

}

bgfx_shaders = {
	selection = {
		includes = [ "common" ]

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
		includes = [ "common" ]

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
			USAMPLER2D(s_selection_map, 0);
			SAMPLER2D(s_selection_depth_map, 1);
			SAMPLER2D(s_depth_map, 2);
			uniform vec4 u_outline_color;

			void main()
			{
				vec2 tex_size = vec2(textureSize(s_selection_map, 0)) - vec2(1, 1);

				uint id[8];
				id[0] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2(-1, -1)), 0).r;
				id[1] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2( 0, -1)), 0).r;
				id[2] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2( 1, -1)), 0).r;
				id[3] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2( 1,  0)), 0).r;
				id[4] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2( 1,  1)), 0).r;
				id[5] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2( 0,  1)), 0).r;
				id[6] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2(-1,  1)), 0).r;
				id[7] = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size + vec2(-1,  0)), 0).r;

				uint ref_id = texelFetch(s_selection_map, ivec2(v_texcoord0 * tex_size), 0).r;

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
				alpha = max(0.6, alpha);

				// Scan the depth around the center and choose the value closest
				// to the viewer. This is to avoid getting depth = 1.0.
				float depth = 1.0;
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2(-1, -1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 0, -1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 1, -1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 1,  0)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 1,  1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 0,  1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2(-1,  1)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2(-1,  0)), 0).r);
				depth = min(depth, texelFetch(s_selection_depth_map, ivec2(v_texcoord0 * tex_size + vec2( 0,  0)), 0).r);

				// Dim alpha if selected object is behind another object.
				if (depth > texelFetch(s_depth_map, ivec2(v_texcoord0 * tex_size), 0).r)
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

}

shaders = {
	selection = {
		bgfx_shader = "selection"
		render_state = "selection"
	}

	outline = {
		bgfx_shader = "outline"
		render_state = "blit"
	}

}

static_compile = [
	{ shader = "selection" defines = [] }
	{ shader = "outline" defines = [] }

]

