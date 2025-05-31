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
			#define map_offt atlas_offset.xy
			#define map_size atlas_offset.z

			float hard_shadow(Sampler _sampler, vec4 shadow_coord, float bias, vec3 atlas_offset)
			{
				vec3 tex_coord = shadow_coord.xyz/shadow_coord.w;

				return shadow2D(_sampler, vec3(tex_coord.xy * map_size + map_offt, tex_coord.z - bias));
			}

			float PCF(Sampler _sampler, vec4 shadow_coord, float bias, vec2 texel_size, vec3 atlas_offset)
			{
				vec2 tex_coord = shadow_coord.xy/shadow_coord.w;
				tex_coord = tex_coord * atlas_offset.z + atlas_offset.xy;

				bool outside = any(greaterThan(tex_coord, map_offt + vec2_splat(map_size)))
							|| any(lessThan   (tex_coord, map_offt))
							 ;

				if (outside)
					return 0.0;

				float result = 0.0;
				vec2 offset = texel_size * shadow_coord.w;

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), bias, atlas_offset);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), bias, atlas_offset);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), bias, atlas_offset);

				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), bias, atlas_offset);
				result += hard_shadow(_sampler, shadow_coord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), bias, atlas_offset);

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
