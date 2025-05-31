include = [
	"core/shaders/common.shader"
	"core/shaders/shadow_map.shader"
]

bgfx_shaders = {
	lighting = {
		includes = [ "shadow_mapping" ]

		code = """
		#if !defined(NO_LIGHT)
		#	define LIGHT_SIZE 4 // In vec4 units.
		#	define MAX_NUM_LIGHTS 32
		#	define MAX_NUM_CASCADES 4
			uniform vec4 u_lights_num;        // num_dir, num_omni, num_spot
			uniform vec4 u_lights_data[LIGHT_SIZE * MAX_NUM_LIGHTS]; // dir_0, .., dir_n-1, omni_0, .., omni_n-1, spot_0, .., spot_n-1
			uniform mat4 u_cascaded_lights[MAX_NUM_CASCADES]; // View-proj-crop matrices for cascaded shadow maps.
			uniform vec4 u_cascaded_texel_size;
			SAMPLER2DSHADOW(u_cascaded_shadow_map, 10);

			CONST(float PI) = 3.14159265358979323846;

			float length_squared(vec3 a)
			{
				return dot(a, a);
			}

			vec3 fresnel(float dot, vec3 f0)
			{
				float a = clamp(1.0 - dot, 0.0, 1.0);
				return f0 + (1.0 - f0) * (a*a*a*a*a);
			}

			float dist_GGX(float ndoth, float roughness)
			{
				float r = roughness*roughness;
				float rr = r*r;
				float d = ((ndoth*ndoth) * (rr - 1.0) + 1.0);
				return rr / (PI * (d*d));
			}

			float geom_schlick_GGX(float ndotv, float roughness)
			{
				float r = roughness + 1.0;
				float k = r*r * (1.0/8.0);
				return ndotv / (ndotv * (1.0 - k) + k);
			}

			float geom_smith(float ndotv, float ndotl, float roughness)
			{
				float ggx2 = geom_schlick_GGX(ndotv, roughness);
				float ggx1 = geom_schlick_GGX(ndotl, roughness);
				return ggx1 * ggx2;
			}

			vec3 calc_radiance(vec3 n, vec3 l, vec3 v, vec3 h, vec3 albedo, vec3 radiance, float metallic, float roughness, vec3 f0)
			{
				float ndotl = max(0.0, dot(n, l));
				float ndotv = max(0.0, dot(n, v));
				float ndoth = max(0.0, dot(n, h));
				float hdotv = max(0.0, dot(h, v));

				vec3 f    = fresnel(hdotv, f0);
				float ndf = dist_GGX(ndoth, roughness);
				float g   = geom_smith(ndotv, ndotl, roughness);

				// Cook-Torrance BRDF.
				vec3 num  = ndf * g * f;
				float den = 4.0 * ndotv * ndotl + 0.0001;
				vec3 specular = num / den;

				vec3 ks = f;
				vec3 kd = (vec3_splat(1.0) - ks) * (1.0 - metallic);

				return (kd * albedo / PI + specular) * radiance * ndotl;
			}

			vec3 calc_dir_light(vec3 n, vec3 v, vec3 color, float intensity, vec3 direction, vec3 albedo, float metallic, float roughness, vec3 f0)
			{
				vec3 l = normalize(-direction); // Direction to light.
				vec3 h = normalize(v + l);      // Half-vector betwen v and l.
				vec3 radiance = color * intensity;
				return calc_radiance(n, l, v, h, albedo, radiance, metallic, roughness, f0);
			}

			vec3 calc_omni_light(vec3 n, vec3 v, vec3 frag_pos, vec3 color, float intensity, vec3 position, float range, vec3 albedo, float metallic, float roughness, vec3 f0)
			{
				vec3 dpos = position - frag_pos;
				vec3 l = normalize(dpos);  // Direction to light.
				vec3 h = normalize(v + l); // Half-vector betwen v and l.
				float dd = length_squared(dpos);
				float attenuation_std = 1.0 / dd;
				float k = 0.15;
				float a = 1.0 - dd / (range*range);
				float t = clamp(a/k, 0.0, 1.0);
				float attenuation = max(mix(0.0, attenuation_std, t), 0.0);
				vec3 radiance = color * intensity * attenuation;
				return calc_radiance(n, l, v, h, albedo, radiance, metallic, roughness, f0);
			}

			vec3 calc_spot_light(vec3 n
				, vec3 v
				, vec3 frag_pos
				, vec3 color
				, float intensity
				, vec3 direction
				, float spot_angle
				, vec3 position
				, float range
				, vec3 albedo
				, float metallic
				, float roughness
				, vec3 f0
				)
			{
				vec3 dpos = position - frag_pos;
				vec3 l = normalize(dpos);  // Direction to light.
				vec3 h = normalize(v + l); // Half-vector betwen v and l.
				float dd = length_squared(dpos);
				float ldotd = dot(l, -direction);
				float r = range / ldotd;
				float rr = r*r;
				float attenuation_std = 1.0 / dd;
				float k = 0.03;

				if (ldotd >= cos(spot_angle) && dd <= rr) {
					float a = ldotd - cos(spot_angle);
					float t = clamp(a/k, 0.0, 1.0);
					float attenuation = mix(0.0, attenuation_std, t);

					vec3 radiance = color * intensity * attenuation;
					return calc_radiance(n, l, v, h, albedo, radiance, metallic, roughness, f0);
				} else {
					return vec3_splat(0.0);
				}
			}

			vec3 calc_lighting(mat3 tbn
				, vec3 n
				, vec3 v
				, vec3 frag_pos
				, vec4 shadow_pos0
				, vec4 shadow_pos1
				, vec4 shadow_pos2
				, vec4 shadow_pos3
				, vec3 albedo
				, float metallic
				, float roughness
				, vec3 f0
				)
			{
				vec3 radiance = vec3_splat(0.0);

				int loffset = 0;
				int num_dir = int(u_lights_num.x);
				int num_omni = int(u_lights_num.y);
				int num_spot = int(u_lights_num.z);

				if (num_dir > 0) {
					// Brightest directional light (index == 0) generates cascaded shadow maps.
					vec3 light_color  = u_lights_data[loffset + 0].rgb;
					float intensity   = u_lights_data[loffset + 0].w;
					vec3 direction    = u_lights_data[loffset + 2].xyz;
					float shadow_bias = u_lights_data[loffset + 3].r;
					float atlas_u     = u_lights_data[loffset + 3].g;
					float atlas_v     = u_lights_data[loffset + 3].b;
					float atlas_size  = u_lights_data[loffset + 3].a;
					loffset += LIGHT_SIZE;

					vec3 local_radiance = calc_dir_light(n
						, v
						, toLinearAccurate(light_color)
						, intensity
						, mul(direction, tbn)
						, albedo
						, metallic
						, roughness
						, f0
						);

					vec2 shadow0 = shadow_pos0.xy/shadow_pos0.w;
					vec2 shadow1 = shadow_pos1.xy/shadow_pos1.w;
					vec2 shadow2 = shadow_pos2.xy/shadow_pos2.w;
					vec2 shadow3 = shadow_pos3.xy/shadow_pos3.w;

					bool atlas0 = all(lessThan(shadow0, vec2_splat(0.99))) && all(greaterThan(shadow0, vec2_splat(0.01)));
					bool atlas1 = all(lessThan(shadow1, vec2_splat(0.99))) && all(greaterThan(shadow1, vec2_splat(0.01)));
					bool atlas2 = all(lessThan(shadow2, vec2_splat(0.99))) && all(greaterThan(shadow2, vec2_splat(0.01)));
					bool atlas3 = all(lessThan(shadow3, vec2_splat(0.99))) && all(greaterThan(shadow3, vec2_splat(0.01)));

					vec2 texel_size = vec2_splat(1.0/u_cascaded_texel_size.x);

					if (atlas0)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos0, shadow_bias, texel_size, vec3(atlas_u             , atlas_v             , atlas_size));
					else if (atlas1)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos1, shadow_bias, texel_size, vec3(atlas_u + atlas_size, atlas_v             , atlas_size));
					else if (atlas2)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos2, shadow_bias, texel_size, vec3(atlas_u             , atlas_v + atlas_size, atlas_size));
					else if (atlas3)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos3, shadow_bias, texel_size, vec3(atlas_u + atlas_size, atlas_v + atlas_size, atlas_size));

					radiance += local_radiance;
				}

				// Others directional lights just add to radiance.
				for (int di = 1; di < num_dir; ++di, loffset += LIGHT_SIZE) {
					vec3 light_color  = u_lights_data[loffset + 0].rgb;
					float intensity   = u_lights_data[loffset + 0].w;
					vec3 direction    = u_lights_data[loffset + 2].xyz;
					float shadow_bias = u_lights_data[loffset + 3].r;
					radiance += calc_dir_light(n
						, v
						, toLinearAccurate(light_color)
						, intensity
						, mul(direction, tbn)
						, albedo
						, metallic
						, roughness
						, f0
						);
				}

				for (int oi = 0; oi < num_omni; ++oi, loffset += LIGHT_SIZE) {
					vec3 light_color  = u_lights_data[loffset + 0].rgb;
					float intensity   = u_lights_data[loffset + 0].w;
					vec3 position     = u_lights_data[loffset + 1].xyz;
					float range       = u_lights_data[loffset + 1].w;
					float shadow_bias = u_lights_data[loffset + 3].r;
					radiance += calc_omni_light(n
						, v
						, frag_pos
						, toLinearAccurate(light_color)
						, intensity
						, mul(position, tbn)
						, range
						, albedo
						, metallic
						, roughness
						, f0
						);
				}

				for (int si = 0; si < num_spot; ++si, loffset += LIGHT_SIZE) {
					vec3 light_color  = u_lights_data[loffset + 0].rgb;
					float intensity   = u_lights_data[loffset + 0].w;
					vec3 position     = u_lights_data[loffset + 1].xyz;
					float range       = u_lights_data[loffset + 1].w;
					vec3 direction    = u_lights_data[loffset + 2].xyz;
					float spot_angle  = u_lights_data[loffset + 2].w;
					float shadow_bias = u_lights_data[loffset + 3].r;
					radiance += calc_spot_light(n
						, v
						, frag_pos
						, toLinearAccurate(light_color)
						, intensity
						, mul(direction, tbn)
						, spot_angle
						, mul(position, tbn)
						, range
						, albedo
						, metallic
						, roughness
						, f0
						);
				}

				return radiance;
			}
		#endif
		"""
	}
}

