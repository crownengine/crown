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
		"""
	}

	shadow = {
		includes = [ "common" ]

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
		#if defined(SKINNING)
				mat4 model;
				model  = a_weight.x * u_model[int(a_indices.x)];
				model += a_weight.y * u_model[int(a_indices.y)];
				model += a_weight.z * u_model[int(a_indices.z)];
				model += a_weight.w * u_model[int(a_indices.w)];
				gl_Position = mul(mul(u_modelViewProj, model), vec4(a_position, 1.0));
		#else
				gl_Position = mul(mul(u_viewProj, u_model[0]), vec4(a_position, 1.0));
		#endif
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
