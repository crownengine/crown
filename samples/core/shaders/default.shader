include = [
	"core/shaders/common.shader"
	"core/shaders/lighting.shader"
]

render_states = {
	debug_line = {
		inherit = "default"
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
		inherit = "opacity"
	}

	blit = {
		inherit = "opacity"
		states = {
			"!defined(BLEND_ENABLED)" = {
				blend_enable = false;
			}
		}
	}

	sprite = {
		inherit = "default"
		states = {
			depth_func = "always"
			blend_enable = true
			blend_src = "src_alpha"
			blend_dst = "inv_src_alpha"
			blend_equation = "add"
		}
	}

	mesh = {
		inherit = "default"
	}

	skydome = {
		inherit = "default"
	}

	noop = {
		inherit = "default"
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
		includes = [ "common" ]

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
		includes = [ "common" ]

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
		includes = [ "common" ]

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
			u_emission_map = { sampler_state = "mirror_anisotropic" }
		}

		varying = """
			vec3 v_normal       : NORMAL    = vec3(0.0, 0.0, 0.0);
			vec3 v_tangent      : TANGENT   = vec3(0.0, 0.0, 0.0);
			vec3 v_bitangent    : BITANGENT = vec3(0.0, 0.0, 0.0);
			vec2 v_texcoord0    : TEXCOORD0 = vec2(0.0, 0.0);
			vec3 v_position     : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
			vec3 v_camera       : TEXCOORD2 = vec3(0.0, 0.0, 0.0);
			vec4 v_shadow0      : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_shadow1      : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_shadow2      : TEXCOORD5 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_shadow3      : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_shadow_local : TEXCOORD7 = vec4(0.0, 0.0, 0.0, 0.0);
			vec3 v_camera_pos   : TEXCOORD8 = vec3(0.0, 0.0, 0.0);

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
			$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_position, v_camera, v_camera_pos, v_shadow0, v_shadow1, v_shadow2, v_shadow3, v_shadow_local
		"""

		vs_code = """
			uniform vec4 u_use_normal_map;
			uniform vec4 u_uv_scale;
			uniform vec4 u_uv_offset;

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

				vec3 normal = decodeNormalUint(a_normal);
				vec3 tangent = decodeNormalUint(a_tangent);
				vec3 bitangent = decodeNormalUint(a_bitangent);
				mat3 normal_matrix = cofactor(model);

				v_position = mul(model, vec4(a_position, 1.0)).xyz;
				v_normal = normalize(mul(normal_matrix, normal)).xyz;
				v_tangent = normalize(mul(normal_matrix, tangent)).xyz;
				v_bitangent = normalize(mul(normal_matrix, bitangent)).xyz;

				mat3 tbn;
				if (u_use_normal_map.r == 1.0)
					tbn = mtxFromCols(v_tangent, v_bitangent, v_normal);
				else
					tbn = mtxFromCols(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

				v_camera_pos = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
				v_camera = mul(v_camera_pos - v_position, tbn);
				v_position = mul(v_position, tbn);

				v_texcoord0 = (a_texcoord0 - vec2_splat(0.5))*u_uv_scale.xy + vec2_splat(0.5) + u_uv_offset.xy;

		#if !defined(NO_LIGHT)
				vec3 pos_offset = a_position + normal * 0.01;
				v_shadow0 = mul(mul(u_cascaded_lights[0], model), vec4(pos_offset, 1.0));
				v_shadow1 = mul(mul(u_cascaded_lights[1], model), vec4(pos_offset, 1.0));
				v_shadow2 = mul(mul(u_cascaded_lights[2], model), vec4(pos_offset, 1.0));
				v_shadow3 = mul(mul(u_cascaded_lights[3], model), vec4(pos_offset, 1.0));
				v_shadow_local = mul(model, vec4(pos_offset, 1.0));
		#endif
			}
		"""

		fs_input_output = """
			$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_position, v_camera, v_camera_pos, v_shadow0, v_shadow1, v_shadow2, v_shadow3, v_shadow_local
		"""

		code = """
		"""

		fs_code = """
			SAMPLER2D(u_albedo_map, 0);
			SAMPLER2D(u_normal_map, 1);
			SAMPLER2D(u_metallic_map, 2);
			SAMPLER2D(u_roughness_map, 3);
			SAMPLER2D(u_ao_map, 4);
			SAMPLER2D(u_emission_map, 5);

			uniform vec4 u_albedo;
			uniform vec4 u_metallic;
			uniform vec4 u_roughness;
			uniform vec4 u_emission_color;
			uniform vec4 u_emission_intensity;
			uniform vec4 u_use_albedo_map;
			uniform vec4 u_use_normal_map;
			uniform vec4 u_use_metallic_map;
			uniform vec4 u_use_roughness_map;
			uniform vec4 u_use_ao_map;
			uniform vec4 u_use_emission_map;

			void main()
			{
				vec3 albedo = u_use_albedo_map.r == 1.0 ? texture2D(u_albedo_map, v_texcoord0).rgb : u_albedo.rgb;

		#if defined(NO_LIGHT)
				vec3 radiance = albedo;
		#else
				vec3 normal;
				if (u_use_normal_map.r == 1.0) {
					normal.xy = texture2DBc5(u_normal_map, v_texcoord0) * 2.0 - 1.0;
					normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy));
				} else {
					normal = v_normal;
				}
				float metallic = u_use_metallic_map.r == 1.0 ? texture2D(u_metallic_map, v_texcoord0).r : u_metallic.r;
				float roughness = u_use_roughness_map.r == 1.0 ? texture2D(u_roughness_map, v_texcoord0).r: u_roughness.r;
				float ao = u_use_ao_map.r == 1.0 ? texture2D(u_ao_map, v_texcoord0).r : 1.0;
				vec3 emission = u_emission_intensity.r * (u_use_emission_map.r == 1.0 ? texture2D(u_emission_map, v_texcoord0).rgb : u_emission_color.rgb);

				mat3 tbn;
				if (u_use_normal_map.r == 1.0)
					tbn = mtxFromCols(v_tangent, v_bitangent, v_normal);
				else
					tbn = mtxFromCols(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

				vec3 n = normalize(normal); // Fragment normal.
				vec3 v = normalize(v_camera); // Versor from fragment to camera pos.
				vec3 f0 = mix(vec3_splat(0.04), albedo, metallic);
				vec3 radiance = calc_lighting(tbn, n, v, v_position, v_camera, v_camera_pos, v_shadow0, v_shadow1, v_shadow2, v_shadow3, v_shadow_local, albedo, metallic, roughness, ao, emission, f0);
		#endif // !defined(NO_LIGHT)

				gl_FragColor = vec4(radiance, 1.0);
			}
		"""
	}

	skydome = {
		includes = [ "common" ]

		samplers = {
			u_skydome_map = { sampler_state = "clamp_anisotropic" }
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
			uniform mat4 u_persp; // Perspective proj.

			void main()
			{
				vec4 world_pos = mul(mul(u_persp, u_modelView), vec4(a_position, 1.0));
				world_pos.z = world_pos.w; // Project to far plane.

				gl_Position = world_pos;
				v_texcoord0 = a_texcoord0;
			}
		"""

		fs_input_output = """
			$input v_texcoord0
		"""

		fs_code = """
			SAMPLER2D(u_skydome_map, 0);
			uniform vec4 u_skydome_intensity;

			void main()
			{
				gl_FragColor = texture2D(u_skydome_map, v_texcoord0) * u_skydome_intensity.x;
			}
		"""
	}

	blit = {
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

			void main()
			{
				gl_FragColor = texture2D(s_color_map, v_texcoord0);
			}
		"""
	}

	fallback = {
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
			void main()
			{
				gl_FragColor = toGammaAccurate(vec4(1.0, 0.0, 1.0, 1.0));
			}
		"""
	}

	noop = {
		includes = [ "common" ]

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

	skydome = {
		bgfx_shader = "skydome"
		render_state = "skydome"
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
	{ shader = "skydome" defines = [] }
	{ shader = "blit" defines = [] }
	{ shader = "blit" defines = ["BLEND_ENABLED"] }
	{ shader = "fallback" defines = [] }
	{ shader = "noop" defines = [] }

]
