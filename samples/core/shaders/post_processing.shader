include = [
	"core/shaders/common.shader"
	"core/shaders/default.shader"
]

render_states = {
	bloom_upsample = {
		inherit = "default"
		states = {
			blend_enable = true
			blend_src = "one"
			blend_dst = "one"
		}
	}

	bloom_combine = {
		inherit = "default"
		states = {
			depth_write_enable = false
		}
	}
}

bgfx_shaders = {
	bloom_params = {
		includes = [ "common" ]

		code = """
			uniform vec4 u_bloom_params;
		#define bloom_weight u_bloom_params.x
		#define bloom_intensity u_bloom_params.y
		#define bloom_threshold u_bloom_params.z
		"""
	}

	bloom_downsample = {
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
			/*
			 * Copyright 2018 Eric Arnebäck. All rights reserved.
			 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
			 */

			SAMPLER2D(s_color_map, 0);

			uniform vec4 u_map_pixel_size; // pixel size of the target texture.

			void main()
			{
				vec2 halfpixel = 0.5 * vec2(u_map_pixel_size.x, u_map_pixel_size.y);
				vec2 oneepixel = 1.0 * vec2(u_map_pixel_size.x, u_map_pixel_size.y);

				vec2 uv = v_texcoord0.xy;

				vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

				sum += (4.0/32.0) * texture2D(s_color_map, uv).rgba;

				sum += (4.0/32.0) * texture2D(s_color_map, uv + vec2(-halfpixel.x, -halfpixel.y) );
				sum += (4.0/32.0) * texture2D(s_color_map, uv + vec2(+halfpixel.x, +halfpixel.y) );
				sum += (4.0/32.0) * texture2D(s_color_map, uv + vec2(+halfpixel.x, -halfpixel.y) );
				sum += (4.0/32.0) * texture2D(s_color_map, uv + vec2(-halfpixel.x, +halfpixel.y) );

				sum += (2.0/32.0) * texture2D(s_color_map, uv + vec2(+oneepixel.x, 0.0) );
				sum += (2.0/32.0) * texture2D(s_color_map, uv + vec2(-oneepixel.x, 0.0) );
				sum += (2.0/32.0) * texture2D(s_color_map, uv + vec2(0.0, +oneepixel.y) );
				sum += (2.0/32.0) * texture2D(s_color_map, uv + vec2(0.0, -oneepixel.y) );

				sum += (1.0/32.0) * texture2D(s_color_map, uv + vec2(+oneepixel.x, +oneepixel.y) );
				sum += (1.0/32.0) * texture2D(s_color_map, uv + vec2(-oneepixel.x, +oneepixel.y) );
				sum += (1.0/32.0) * texture2D(s_color_map, uv + vec2(+oneepixel.x, -oneepixel.y) );
				sum += (1.0/32.0) * texture2D(s_color_map, uv + vec2(-oneepixel.x, -oneepixel.y) );

				gl_FragColor.xyzw = sum;
			}
		"""
	}

	bloom_upsample = {
		includes = [ "common" "bloom_params" ]

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
			/*
			 * Copyright 2018 Eric Arnebäck. All rights reserved.
			 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
			 */
			SAMPLER2D(s_color_map, 0);
			uniform vec4 u_map_pixel_size;

			void main()
			{
				vec2 halfpixel = u_map_pixel_size.xy;
				vec2 uv = v_texcoord0.xy;

				vec4 sum = vec4_splat(0.0);

				sum += (2.0 / 16.0) * texture2D(s_color_map, uv + vec2(-halfpixel.x,  0.0) );
				sum += (2.0 / 16.0) * texture2D(s_color_map, uv + vec2( 0.0,          halfpixel.y) );
				sum += (2.0 / 16.0) * texture2D(s_color_map, uv + vec2( halfpixel.x,  0.0) );
				sum += (2.0 / 16.0) * texture2D(s_color_map, uv + vec2( 0.0,         -halfpixel.y) );

				sum += (1.0 / 16.0) * texture2D(s_color_map, uv + vec2(-halfpixel.x, -halfpixel.y) );
				sum += (1.0 / 16.0) * texture2D(s_color_map, uv + vec2(-halfpixel.x,  halfpixel.y) );
				sum += (1.0 / 16.0) * texture2D(s_color_map, uv + vec2( halfpixel.x, -halfpixel.y) );
				sum += (1.0 / 16.0) * texture2D(s_color_map, uv + vec2( halfpixel.x,  halfpixel.y) );

				sum += (4.0 / 16.0) * texture2D(s_color_map, uv);

				gl_FragColor = bloom_intensity * sum;
			}
		"""
	}

	bloom_combine = {
		includes = [ "common" "bloom_params" ]

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
			SAMPLER2D(s_color_map, 0);
			SAMPLER2D(s_bloom_map, 1);

			void main()
			{
				vec3 color = texture2D(s_color_map, v_texcoord0).rgb;
				vec3 bloom = texture2D(s_bloom_map, v_texcoord0).rgb;
				gl_FragColor = vec4(mix(color, bloom, bloom_weight), 1.0);
			}
		"""
	}

	tonemap = {
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
			SAMPLER2D(s_color_map, 0);
			uniform vec4 u_tonemap_type;

			void main()
			{
				vec4 color = vec4(texture2D(s_color_map, v_texcoord0).rgb, 1.0);

				if (u_tonemap_type.x == 0.0)
					gl_FragColor = toGammaAccurate(color);
				else if (u_tonemap_type.x == 1.0)
					gl_FragColor = toReinhard(color);
				else if (u_tonemap_type.x == 2.0)
					gl_FragColor = toFilmic(color);
				else if (u_tonemap_type.x == 3.0)
					gl_FragColor = toAcesFilmic(color);
				else
					gl_FragColor = color;
			}
		"""
	}

}

shaders = {
	bloom_downsample = {
		bgfx_shader = "bloom_downsample"
		render_state = "default"
	}

	bloom_upsample = {
		bgfx_shader = "bloom_upsample"
		render_state = "bloom_upsample"
	}

	bloom_combine = {
		bgfx_shader = "bloom_combine"
		render_state = "bloom_combine"
	}

	tonemap = {
		bgfx_shader = "tonemap"
		render_state = "blit"
	}

}

static_compile = [
	{ shader = "bloom_downsample" defines = [] }
	{ shader = "bloom_upsample" defines = [] }
	{ shader = "bloom_combine" defines = [] }
	{ shader = "tonemap" defines = [] }

]

