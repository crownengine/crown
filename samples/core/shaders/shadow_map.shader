include = [ "core/shaders/common.shader" ]

render_states = {
	shadow = {
		states = {
			rgb_write_enable = false
			alpha_write_enable = false
			depth_write_enable = true
		}
	}
}

bgfx_shaders = {
	shadow_mapping = {
		includes = [ "common" ]

		code = """
		#define Sampler sampler2DShadow

			float hard_shadow(Sampler _sampler, vec4 shadow_coord, float bias)
			{
				vec3 tex_coord = shadow_coord.xyz/shadow_coord.w;

				return shadow2D(_sampler, vec3(tex_coord.xy, tex_coord.z - bias));
			}

			float PCF4(Sampler _sampler, vec4 shadow_coord, float bias, vec2 texel_size)
			{
				float result = 0.0;
				vec2 offset = texel_size * shadow_coord.w;

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 0.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 0.5,  0.5) * offset, 0.0, 0.0), bias);

				return result / 4.0;
			}

			float PCF9(Sampler _sampler, vec4 shadow_coord, float bias, vec2 texel_size)
			{
				float result = 0.0;
				vec2 offset = texel_size * shadow_coord.w;

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.0, -1.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.0,  0.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.0,  1.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 0.0, -1.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord, bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 0.0,  1.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 1.0, -1.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 1.0,  0.0) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2( 1.0,  1.0) * offset, 0.0, 0.0), bias);

				return result / 9.0;
			}

			float PCF(Sampler _sampler, vec4 shadow_coord, float bias, vec2 texel_size)
			{
				float result = 0.0;
				vec2 offset = texel_size * shadow_coord.w;

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), bias);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), bias);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), bias);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), bias);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), bias);

				return result / 16.0;
			}

			float shadow(Sampler _sampler, vec4 shadow_coord, float bias, vec2 texel_size, float samples)
			{
				if (samples < 2.0)
					return hard_shadow(_sampler, shadow_coord, bias);
				else if (samples < 5.0)
					return PCF4(_sampler, shadow_coord, bias, texel_size);
				else if (samples < 10.0)
					return PCF9(_sampler, shadow_coord, bias, texel_size);

				return PCF(_sampler, shadow_coord, bias, texel_size);
			}
		"""
	}

	shadow = {
		includes = [ "common" "skinning" ]

		varying = """
			vec3 a_position : POSITION;
			vec4 a_indices  : BLENDINDICES;
			vec4 a_weight   : BLENDWEIGHT;
		"""

		vs_input_output = """
		#if defined(SKINNING)
			$input a_position, a_indices, a_weight
		#else
			$input a_position
		#endif
		"""

		vs_code = """
			void main()
			{
				gl_Position = mul(u_modelViewProj, skin(a_position));
			}
		"""

		fs_input_output = """
		"""

		fs_code = """
			void main()
			{
				gl_FragColor = vec4_splat(0.0);
			}
		"""
	}
}

shaders = {
	shadow = {
		bgfx_shader = "shadow"
		render_state = "shadow"
	}
}

static_compile = [
	{ shader = "shadow" defines = [] }
	{ shader = "shadow" defines = ["SKINNING"] }

]
