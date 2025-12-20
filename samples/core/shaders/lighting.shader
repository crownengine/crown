include = [
	"core/shaders/common.shader"
	"core/shaders/shadow_map.shader"
]

bgfx_shaders = {
	lighting = {
		includes = [ "shadow_mapping" "fog" ]

		code = """
		#if !defined(NO_LIGHT)
		#	define LIGHT_SIZE 22 // In vec4 units.
		#	define MAX_NUM_LIGHTS 32
		#	define MAX_NUM_CASCADES 4
			uniform vec4 u_lights_num;        // num_dir, num_omni, num_spot
			SAMPLER2D(u_lights_data, 12); // dir_0, .., dir_n-1, omni_0, .., omni_n-1, spot_0, .., spot_n-1
			uniform mat4 u_cascaded_lights[MAX_NUM_CASCADES]; // View-proj-crop matrices for cascaded shadow maps.
			uniform vec4 u_shadow_maps_texel_sizes;
		#	define sun_sm_texel_size u_shadow_maps_texel_sizes.xy
		#	define local_lights_sm_texel_size u_shadow_maps_texel_sizes.zw
			SAMPLER2DSHADOW(u_cascaded_shadow_map, 10);
			SAMPLER2DSHADOW(u_local_lights_shadow_map, 11);
			uniform vec4 u_local_lights_params;
		#	define local_lights_distance_culling u_local_lights_params.x
		#	define local_lights_distance_culling_fade u_local_lights_params.y
		#	define local_lights_distance_culling_cutoff u_local_lights_params.z
			uniform vec4 u_lighting_params;
		#	define ambient_color u_lighting_params.xyz

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

			float fade_smoothstep(float camera_dist, float fade_dist, float cutoff_dist)
			{
				if (cutoff_dist == fade_dist)
					return camera_dist <= fade_dist ? 1.0 : 0.0;

				return 1.0 - smoothstep(fade_dist, cutoff_dist, camera_dist);
			}

			vec3 apply_distance_fading(vec3 radiance, vec3 light_pos, vec3 camera_pos)
			{
				if (local_lights_distance_culling == 0.0)
					return radiance;

				float fade_dist   = local_lights_distance_culling_fade;
				float cutoff_dist = local_lights_distance_culling_cutoff;
				float camera_dist = length(light_pos - camera_pos);

				return mix(vec3_splat(0.0), radiance, fade_smoothstep(camera_dist, fade_dist, cutoff_dist));
			}

			vec4 lights_data(int offset)
			{
				float u = (float(offset) + 0.5) / float(LIGHT_SIZE * MAX_NUM_LIGHTS);
				return texture2D(u_lights_data, vec2(u, 0.0));
			}

			vec3 calc_lighting(mat3 tbn
				, vec3 n
				, vec3 v
				, vec3 frag_pos
				, vec3 camera_frag_pos
				, vec3 camera_pos
				, vec4 shadow_pos0
				, vec4 shadow_pos1
				, vec4 shadow_pos2
				, vec4 shadow_pos3
				, vec4 shadow_local
				, vec3 albedo
				, float metallic
				, float roughness
				, float ao
				, vec3 emission
				, vec3 f0
				)
			{
				vec3 radiance = ao * toLinearAccurate(ambient_color) * albedo;

				int loffset = 0;
				int num_dir = int(u_lights_num.x);
				int num_omni = int(u_lights_num.y);
				int num_spot = int(u_lights_num.z);

				vec3 sun_color = vec3(1, 1, 1);

				if (num_dir > 0) {
					// Brightest directional light (index == 0) generates cascaded shadow maps.
					vec3 light_color  = lights_data(loffset +  0).rgb;
					float intensity   = lights_data(loffset +  0).w;
					vec3 direction    = lights_data(loffset +  2).xyz;
					vec4 atlas_u      = lights_data(loffset + 19).xyzw;
					vec4 atlas_v      = lights_data(loffset + 20).xyzw;
					float atlas_size  = lights_data(loffset + 21).x;
					float shadow_bias = lights_data(loffset + 21).y;
					loffset += LIGHT_SIZE;

					sun_color = light_color;

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

					bool atlas0 = all(lessThan(shadow0, vec2_splat(1.0))) && all(greaterThan(shadow0, vec2_splat(0.0)));
					bool atlas1 = all(lessThan(shadow1, vec2_splat(1.0))) && all(greaterThan(shadow1, vec2_splat(0.0)));
					bool atlas2 = all(lessThan(shadow2, vec2_splat(1.0))) && all(greaterThan(shadow2, vec2_splat(0.0)));
					bool atlas3 = all(lessThan(shadow3, vec2_splat(1.0))) && all(greaterThan(shadow3, vec2_splat(0.0)));

					if (atlas0)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos0, shadow_bias, sun_sm_texel_size, vec3(atlas_u.x             , atlas_v.x             , atlas_size));
					else if (atlas1)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos1, shadow_bias, sun_sm_texel_size, vec3(atlas_u.x + atlas_size, atlas_v.x             , atlas_size));
					else if (atlas2)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos2, shadow_bias, sun_sm_texel_size, vec3(atlas_u.x             , atlas_v.x + atlas_size, atlas_size));
					else if (atlas3)
						local_radiance *= PCF(u_cascaded_shadow_map, shadow_pos3, shadow_bias, sun_sm_texel_size, vec3(atlas_u.x + atlas_size, atlas_v.x + atlas_size, atlas_size));

					radiance += local_radiance;
				}

				// Others directional lights just add to radiance.
				for (int di = 1; di < num_dir; ++di, loffset += LIGHT_SIZE) {
					vec3 light_color  = lights_data(loffset + 0).rgb;
					float intensity   = lights_data(loffset + 0).w;
					vec3 direction    = lights_data(loffset + 2).xyz;
					float shadow_bias = lights_data(loffset + 21).y;

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
					vec3 light_color  = lights_data(loffset + 0).rgb;
					float intensity   = lights_data(loffset + 0).w;
					vec3 position     = lights_data(loffset + 1).xyz;
					float range       = lights_data(loffset + 1).w;
					float cast_shadow = lights_data(loffset + 21).z;

					vec3 local_radiance = calc_omni_light(n
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

					if (cast_shadow == 1.0) {
						// Tetrahedron normals.
						CONST(vec3 bn) = vec3(        0.0f,  0.81649661f, -0.57735026f);
						CONST(vec3 yn) = vec3(        0.0f, -0.81649661f, -0.57735026f);
						CONST(vec3 gn) = vec3(-0.81649661f,  0.0f,         0.57735026f);
						CONST(vec3 rn) = vec3( 0.81649661f,  0.0f,         0.57735026f);

						vec3 sl = shadow_local.xyz - position; // Transform to light-local space.

						// Select tetrahedon face.
						float b = dot(sl, bn);
						float y = dot(sl, yn);
						float g = dot(sl, gn);
						float r = dot(sl, rn);
						float maximum = max(max(b, y), max(g, r));

						vec4 atlas_u      = lights_data(loffset + 19);
						vec4 atlas_v      = lights_data(loffset + 20);
						float atlas_size  = lights_data(loffset + 21).x;
						float shadow_bias = lights_data(loffset + 21).y;

						vec4 shadow_pos0;
						vec3 col;
						vec3 atlas_offset;

						if (maximum == b) {
							// Tetrahedron mvp matrices.
							mat4 bmtx = mtxFromCols(lights_data(loffset + 3)
								, lights_data(loffset + 4)
								, lights_data(loffset + 5)
								, lights_data(loffset + 6)
								);
							shadow_pos0 = mul(bmtx, shadow_local);
							col = vec3(0.1, 0.1, 1);
							atlas_offset = vec3(atlas_u.x, atlas_v.x, atlas_size);
						} else if (maximum == y) {
							mat4 ymtx = mtxFromCols(lights_data(loffset + 7)
								, lights_data(loffset + 8)
								, lights_data(loffset + 9)
								, lights_data(loffset + 10)
								);
							shadow_pos0 = mul(ymtx, shadow_local);
							col = vec3(1, 1, 0);
							atlas_offset = vec3(atlas_u.y, atlas_v.y, atlas_size);
						} else if (maximum == g) {
							mat4 gmtx = mtxFromCols(lights_data(loffset + 11)
								, lights_data(loffset + 12)
								, lights_data(loffset + 13)
								, lights_data(loffset + 14)
								);
							shadow_pos0 = mul(gmtx, shadow_local);
							col = vec3(0, 1, 0);
							atlas_offset = vec3(atlas_u.z, atlas_v.z, atlas_size);
						} else {
							mat4 rmtx = mtxFromCols(lights_data(loffset + 15)
								, lights_data(loffset + 16)
								, lights_data(loffset + 17)
								, lights_data(loffset + 18)
								);
							shadow_pos0 = mul(rmtx, shadow_local);
							col = vec3(1, 0, 0);
							atlas_offset = vec3(atlas_u.w, atlas_v.w, atlas_size);
						}

						local_radiance *= PCF(u_local_lights_shadow_map
							, shadow_pos0
							, shadow_bias
							, local_lights_sm_texel_size / 4.0
							, atlas_offset
							);
					}

					radiance += apply_distance_fading(local_radiance, position, camera_pos);
				}

				for (int si = 0; si < num_spot; ++si, loffset += LIGHT_SIZE) {
					vec3 light_color  = lights_data(loffset + 0).rgb;
					float intensity   = lights_data(loffset + 0).w;
					vec3 position     = lights_data(loffset + 1).xyz;
					float range       = lights_data(loffset + 1).w;
					vec3 direction    = lights_data(loffset + 2).xyz;
					float spot_angle  = lights_data(loffset + 2).w;
					float cast_shadow = lights_data(loffset + 21).z;

					vec3 local_radiance = calc_spot_light(n
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

					if (cast_shadow == 1.0) {
						mat4 mvp = mtxFromCols(lights_data(loffset + 3)
							, lights_data(loffset + 4)
							, lights_data(loffset + 5)
							, lights_data(loffset + 6)
							);
						vec4 atlas_u      = lights_data(loffset + 19);
						vec4 atlas_v      = lights_data(loffset + 20);
						float atlas_size  = lights_data(loffset + 21).x;
						float shadow_bias = lights_data(loffset + 21).y;

						local_radiance *= PCF(u_local_lights_shadow_map
							, mul(mvp, shadow_local)
							, shadow_bias
							, local_lights_sm_texel_size
							, vec3(atlas_u.x, atlas_v.x, atlas_size)
							);
					}

					radiance += apply_distance_fading(local_radiance, position, camera_pos);
				}

				return apply_fog(emission + radiance, length(camera_frag_pos), sun_color);
			}
		#endif
		"""
	}

	fog = {
		code = """
			uniform vec4 u_fog_data[2];

		#define fog_color     u_fog_data[0].rgb
		#define fog_density   u_fog_data[0].w
		#define fog_range_min u_fog_data[1].x
		#define fog_range_max u_fog_data[1].y
		#define fog_sun_blend u_fog_data[1].z
		#define fog_enabled   u_fog_data[1].w

			vec3 apply_fog(vec3 radiance, float d, vec3 sun_color)
			{
				if (fog_enabled == 0.0)
					return radiance;

				if (d < fog_range_min || d > fog_range_max)
					return radiance;

				float d2 = d - fog_range_min;
				float f = exp(-fog_density * d2);
				return mix(mix(fog_color, sun_color, fog_sun_blend), radiance, f);
			}
		"""
	}
}
