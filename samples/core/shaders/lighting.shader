include = ["core/shaders/common.shader"]

bgfx_shaders = {
	lighting = {
		code = """
		#if !defined(NO_LIGHT)
			uniform vec4 u_lights_num;        // num_dir, num_omni, num_spot
			uniform vec4 u_lights_data[3*32]; // dir_0, .., dir_n-1, omni_0, .., omni_n-1, spot_0, .., spot_n-1

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

			vec3 calc_lighting(mat3 tbn, vec3 n, vec3 v, vec3 frag_pos, vec3 albedo, float metallic, float roughness, vec3 f0)
			{
				vec3 radiance = vec3_splat(0.0);

				int loffset = 0;
				int num_dir = int(u_lights_num.x);
				int num_omni = int(u_lights_num.y);
				int num_spot = int(u_lights_num.z);

				for (int di = 0; di < num_dir; ++di, loffset += 3) {
					vec3 light_color = u_lights_data[loffset + 0].rgb;
					float intensity  = u_lights_data[loffset + 0].w;
					vec3 direction   = u_lights_data[loffset + 2].xyz;
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

				for (int oi = 0; oi < num_omni; ++oi, loffset += 3) {
					vec3 light_color = u_lights_data[loffset + 0].rgb;
					float intensity  = u_lights_data[loffset + 0].w;
					vec3 position    = u_lights_data[loffset + 1].xyz;
					float range      = u_lights_data[loffset + 1].w;
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

				for (int si = 0; si < num_spot; ++si, loffset += 3) {
					vec3 light_color = u_lights_data[loffset + 0].rgb;
					float intensity  = u_lights_data[loffset + 0].w;
					vec3 position    = u_lights_data[loffset + 1].xyz;
					float range      = u_lights_data[loffset + 1].w;
					vec3 direction   = u_lights_data[loffset + 2].xyz;
					float spot_angle = u_lights_data[loffset + 2].w;
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

