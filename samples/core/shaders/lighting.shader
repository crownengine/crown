include = [
	"core/shaders/common.shader"
	"core/shaders/shadow_map.shader"
]

bgfx_shaders = {
	lighting = {
		includes = [ "shadow_mapping" ]

		code = """
		#if !defined(NO_LIGHT)
		#	define LIGHT_SIZE 24 // In vec4 units.
		#	define MAX_NUM_LIGHTS 32
		#	define MAX_NUM_CASCADES 4
			uniform vec4 u_lights_num;             // num_dir, num_omni, num_spot
		#	if BGFX_SHADER_LANGUAGE_GLSL
			uniform highp sampler2D u_lights_data; // See SAMPLER2D(u_lights_data).
		#	else
			SAMPLER2D(u_lights_data, 12);          // dir_0, .., dir_n-1, omni_0, .., omni_n-1, spot_0, .., spot_n-1
		#	endif
			uniform mat4 u_cascaded_lights[MAX_NUM_CASCADES]; // View-proj-crop matrices for cascaded shadow maps.
			uniform vec4 u_cascade_shadow_texel_size;
			uniform vec4 u_shadow_map_params[2];
		#	define sun_sm_texel_size u_shadow_map_params[0].xy
		#	define local_lights_sm_texel_size u_shadow_map_params[0].zw
		#	define sun_shadow_map_samples u_shadow_map_params[1].x
		#	define local_lights_shadow_map_samples u_shadow_map_params[1].y
			SAMPLER2DSHADOW(u_cascaded_shadow_map, 10);
			SAMPLER2DSHADOW(u_local_lights_shadow_map, 11);
			SAMPLER2D(u_lights_cookie_atlas, 13);
			uniform vec4 u_local_lights_params;
		#	define local_lights_distance_culling u_local_lights_params.x
		#	define local_lights_distance_culling_fade u_local_lights_params.y
		#	define local_lights_distance_culling_cutoff u_local_lights_params.z
			uniform vec4 u_lighting_params;
		#	define ambient_color u_lighting_params.xyz
		#	define shadow_distance u_lighting_params.w

			float length_squared(vec3 a)
			{
				return dot(a, a);
			}

			float smooth_range_fade(float dd, float rr, float fade_width)
			{
				float a = 1.0 - dd / rr;
				return smoothstep(0.0, fade_width, a);
			}

			vec3 fresnel(float dot, vec3 f0)
			{
				float a = 1.0 - dot;
				return f0 + (1.0 - f0) * (a*a*a*a*a);
			}

			float dist_GGX(float ndoth, float roughness)
			{
				float r = roughness*roughness;
				float rr = r*r;
				float d = 1.0 - ndoth*ndoth + ndoth*ndoth*rr;
				float a = r/d;
				return a*a * (1.0/M_PI);
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
				float ndoth = clamp(dot(n, h), 0.0, 1.0);
				float hdotv = clamp(dot(h, v), 0.0, 1.0);

				vec3 f    = fresnel(hdotv, f0);
				float ndf = dist_GGX(ndoth, roughness);
				float g   = geom_smith(ndotv, ndotl, roughness);

				// Cook-Torrance BRDF.
				vec3 num  = ndf * g * f;
				float den = 4.0 * ndotv * ndotl + 0.0001;
				vec3 specular = num / den;

				vec3 ks = f;
				vec3 kd = (vec3_splat(1.0) - ks) * (1.0 - metallic);

				return (kd * albedo / M_PI + specular) * radiance * ndotl;
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
				highp vec3 dpos = position - frag_pos;
				vec3 l = normalize(dpos);  // Direction to light.
				vec3 h = normalize(v + l); // Half-vector betwen v and l.
				float dd = length_squared(dpos);
				float attenuation_std = 1.0 / max(dd, 0.0001);
				float range_fade = smooth_range_fade(dd, range*range, 0.15);
				float attenuation = attenuation_std * range_fade;
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
				highp vec3 dpos = position - frag_pos;
				vec3 l = normalize(dpos);  // Direction to light.
				vec3 h = normalize(v + l); // Half-vector betwen v and l.
				float dd = length_squared(dpos);
				float ldotd = dot(l, -direction);
				float attenuation_std = 1.0 / max(dd, 0.0001);
				float k = 0.03;

				if (ldotd >= cos(spot_angle)) {
					float r = range / ldotd;
					float rr = r*r;
					float a = ldotd - cos(spot_angle);
					float t = clamp(a/k, 0.0, 1.0);
					float range_fade = smooth_range_fade(dd, rr, 0.15);
					float attenuation = attenuation_std * t * range_fade;

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

			vec4 atlas_shadow_coord(vec4 shadow_coord, vec3 atlas_offset)
			{
				vec4 coord = shadow_coord;
				coord.xy = shadow_coord.xy * atlas_offset.z + atlas_offset.xy * shadow_coord.w;
				return coord;
			}

			vec3 shadow_bias_position(vec3 world_pos
				, vec3 geometric_n
				, vec3 light_dir
				, float world_texel_size
				, float normal_bias
				)
			{
				float slope = 1.0 - saturate(dot(geometric_n, light_dir));
				return world_pos + geometric_n * normal_bias * slope * world_texel_size;
			}

			bool shadow_coord_inside_atlas_tile(vec4 atlas_shadow_coord, vec3 atlas_offset)
			{
				vec2 tex_coord = atlas_shadow_coord.xy/atlas_shadow_coord.w;
				return all(lessThan(tex_coord, atlas_offset.xy + vec2_splat(atlas_offset.z)))
					&& all(greaterThan(tex_coord, atlas_offset.xy))
					;
			}

			vec3 sample_light_cookie(vec4 rect, vec2 uv, vec4 transform, float center, float flip_y)
			{
				uv = (uv - center) * transform.w + center + transform.yz;
				uv.y = flip_y > 0.0 ? 1.0 - uv.y : uv.y;
				vec2 atlas_uv = mix(rect.xy, rect.zw, fract(uv));
				return texture2D(u_lights_cookie_atlas, atlas_uv).rgb;
			}

			vec2 equirectangular_uv(vec3 direction)
			{
				return vec2(atan2(direction.x, direction.z) * (0.5 / M_PI) + 0.5
					, acos(clamp(direction.y, -1.0, 1.0)) / M_PI
					);
			}

			vec4 lights_data(int offset)
			{
				float u = (float(offset) + 0.5) / float(LIGHT_SIZE * MAX_NUM_LIGHTS);
				return texture2D(u_lights_data, vec2(u, 0.0));
			}

			vec3 calc_lighting(mat3 tbn
				, vec3 n
				, vec3 geometric_n
				, vec3 v
				, vec3 frag_pos
				, vec3 camera_frag_pos
				, vec3 camera_pos
				, vec3 world_pos
				, vec3 albedo
				, float metallic
				, float roughness
				, float ao
				, vec3 emission
				, vec3 f0
				)
			{
				// Keep roughness^4, used in dist_GGX(), in the normalized FP16 range. FP16 smallest
				// normalized value is 2^-14, requiring: roughness^4 >= 2^-14 ~= 0.089
				roughness = max(roughness, 0.089);

				vec3 radiance = ao * toLinearAccurate(ambient_color) * albedo;
				float camera_distance = length(camera_frag_pos);
				bool receive_shadow = camera_distance <= shadow_distance;

				int loffset = 0;
				int num_dir = int(u_lights_num.x);
				int num_omni = int(u_lights_num.y);
				int num_spot = int(u_lights_num.z);

				if (num_dir > 0) {
					// Brightest directional light (index == 0) generates cascaded shadow maps and supports a cookie.
					vec3 light_color  = lights_data(loffset +  0).rgb;
					float intensity   = lights_data(loffset +  0).w;
					vec3 direction    = lights_data(loffset +  2).xyz;
					vec4 light_params = lights_data(loffset + 3);
					bool cast_shadow  = light_params.x == 1.0;
					bool has_cookie   = light_params.y == 1.0;

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

					if (receive_shadow && cast_shadow) {
						vec4 shadow0_near = lights_data(loffset +  4);
						vec4 shadow0_far  = lights_data(loffset +  5);
						vec4 shadow1_near = lights_data(loffset +  6);
						vec4 shadow1_far  = lights_data(loffset +  7);
						vec4 shadow2_near = lights_data(loffset +  8);
						vec4 shadow2_far  = lights_data(loffset +  9);
						vec4 shadow3_near = lights_data(loffset + 10);
						vec4 shadow3_far  = lights_data(loffset + 11);
						vec2 atlas_offset = lights_data(loffset + 20).xy;
						float atlas_size  = lights_data(loffset + 20).z;
						float shadow_bias = light_params.z;
						float normal_bias = light_params.w;
						vec3 shadow_world = world_pos;
						vec4 shadow_pos0 = mul(u_cascaded_lights[0], vec4(shadow_world, 1.0));
						vec4 shadow_pos1 = mul(u_cascaded_lights[1], vec4(shadow_world, 1.0));
						vec4 shadow_pos2 = mul(u_cascaded_lights[2], vec4(shadow_world, 1.0));
						vec4 shadow_pos3 = mul(u_cascaded_lights[3], vec4(shadow_world, 1.0));
						vec3 atlas_offset0 = vec3(atlas_offset, atlas_size);
						vec3 atlas_offset1 = vec3(atlas_offset + vec2(atlas_size, 0.0), atlas_size);
						vec3 atlas_offset2 = vec3(atlas_offset + vec2(0.0, atlas_size), atlas_size);
						vec3 atlas_offset3 = vec3(atlas_offset + vec2_splat(atlas_size), atlas_size);
						vec4 atlas_shadow_pos0 = atlas_shadow_coord(shadow_pos0, atlas_offset0);
						vec4 atlas_shadow_pos1 = atlas_shadow_coord(shadow_pos1, atlas_offset1);
						vec4 atlas_shadow_pos2 = atlas_shadow_coord(shadow_pos2, atlas_offset2);
						vec4 atlas_shadow_pos3 = atlas_shadow_coord(shadow_pos3, atlas_offset3);

						// All near planes have the same normal and all far planes have its opposite.
						float shadow_depth = dot(shadow0_near.xyz, shadow_world);
						bool atlas0 = shadow_depth >= shadow0_near.w && shadow_depth <= -shadow0_far.w && shadow_coord_inside_atlas_tile(atlas_shadow_pos0, atlas_offset0);
						bool atlas1 = shadow_depth >= shadow1_near.w && shadow_depth <= -shadow1_far.w && shadow_coord_inside_atlas_tile(atlas_shadow_pos1, atlas_offset1);
						bool atlas2 = shadow_depth >= shadow2_near.w && shadow_depth <= -shadow2_far.w && shadow_coord_inside_atlas_tile(atlas_shadow_pos2, atlas_offset2);
						bool atlas3 = shadow_depth >= shadow3_near.w && shadow_depth <= -shadow3_far.w && shadow_coord_inside_atlas_tile(atlas_shadow_pos3, atlas_offset3);
						vec3 shadow_light_dir = normalize(-direction);

						if (atlas0) {
							vec3 biased_world_pos0 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.x, normal_bias);
							vec4 biased_shadow_pos0 = mul(u_cascaded_lights[0], vec4(biased_world_pos0, 1.0));
							float shadow0 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos0, atlas_offset0), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
							if (atlas1 && shadow_depth > shadow1_near.w) {
								vec3 biased_world_pos1 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.y, normal_bias);
								vec4 biased_shadow_pos1 = mul(u_cascaded_lights[1], vec4(biased_world_pos1, 1.0));
								float shadow1 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos1, atlas_offset1), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
								float blend = (shadow_depth - shadow1_near.w) * rcp(-shadow0_far.w - shadow1_near.w);
								shadow0 = mix(shadow0, shadow1, blend);
							}
							local_radiance *= shadow0;
						} else if (atlas1) {
							vec3 biased_world_pos1 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.y, normal_bias);
							vec4 biased_shadow_pos1 = mul(u_cascaded_lights[1], vec4(biased_world_pos1, 1.0));
							float shadow1 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos1, atlas_offset1), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
							if (atlas2 && shadow_depth > shadow2_near.w) {
								vec3 biased_world_pos2 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.z, normal_bias);
								vec4 biased_shadow_pos2 = mul(u_cascaded_lights[2], vec4(biased_world_pos2, 1.0));
								float shadow2 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos2, atlas_offset2), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
								float blend = (shadow_depth - shadow2_near.w) * rcp(-shadow1_far.w - shadow2_near.w);
								shadow1 = mix(shadow1, shadow2, blend);
							}
							local_radiance *= shadow1;
						} else if (atlas2) {
							vec3 biased_world_pos2 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.z, normal_bias);
							vec4 biased_shadow_pos2 = mul(u_cascaded_lights[2], vec4(biased_world_pos2, 1.0));
							float shadow2 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos2, atlas_offset2), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
							if (atlas3 && shadow_depth > shadow3_near.w) {
								vec3 biased_world_pos3 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.w, normal_bias);
								vec4 biased_shadow_pos3 = mul(u_cascaded_lights[3], vec4(biased_world_pos3, 1.0));
								float shadow3 = shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos3, atlas_offset3), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
								float blend = (shadow_depth - shadow3_near.w) * rcp(-shadow2_far.w - shadow3_near.w);
								shadow2 = mix(shadow2, shadow3, blend);
							}
							local_radiance *= shadow2;
						} else if (atlas3) {
							vec3 biased_world_pos3 = shadow_bias_position(shadow_world, geometric_n, shadow_light_dir, u_cascade_shadow_texel_size.w, normal_bias);
							vec4 biased_shadow_pos3 = mul(u_cascaded_lights[3], vec4(biased_world_pos3, 1.0));
							local_radiance *= shadow(u_cascaded_shadow_map, atlas_shadow_coord(biased_shadow_pos3, atlas_offset3), shadow_bias, sun_sm_texel_size, sun_shadow_map_samples);
						}
					}

					if (has_cookie) {
						// .x is the scale (world-space size in meters of one cookie
						// tile), .yz is the (x, y) offset of the cookie pattern.
						vec4 cookie_transform = lights_data(loffset + 21);
						vec4 cookie_up = lights_data(loffset + 22);
						vec4 cookie_rect = lights_data(loffset + 23);
						vec3 cookie_right = cross(direction, cookie_up.xyz);
						vec2 cookie_uv = vec2(dot(world_pos, cookie_right)
							, dot(world_pos, cookie_up.xyz)
							);
						local_radiance *= sample_light_cookie(cookie_rect, cookie_uv, cookie_transform, 0.0, cookie_up.w);
					}

					radiance += local_radiance;
					loffset += LIGHT_SIZE;
				}

				// Others directional lights just add to radiance.
				for (int di = 1; di < num_dir; ++di, loffset += LIGHT_SIZE) {
					vec3 light_color  = lights_data(loffset + 0).rgb;
					float intensity   = lights_data(loffset + 0).w;
					vec3 direction    = lights_data(loffset + 2).xyz;

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
					vec3 direction    = lights_data(loffset + 2).xyz;
					vec4 light_params = lights_data(loffset + 3);
					bool cast_shadow  = light_params.x == 1.0;
					bool has_cookie   = light_params.y == 1.0;
					vec3 light_pos    = mul(position, tbn);

					vec3 local_radiance = calc_omni_light(n
						, v
						, frag_pos
						, toLinearAccurate(light_color)
						, intensity
						, light_pos
						, range
						, albedo
						, metallic
						, roughness
						, f0
						);

					if (receive_shadow && cast_shadow) {
						// Tetrahedron normals.
						CONST(vec3 bn) = vec3(        0.0f,  0.81649661f, -0.57735026f);
						CONST(vec3 yn) = vec3(        0.0f, -0.81649661f, -0.57735026f);
						CONST(vec3 gn) = vec3(-0.81649661f,  0.0f,         0.57735026f);
						CONST(vec3 rn) = vec3( 0.81649661f,  0.0f,         0.57735026f);

						vec2 atlas_base   = lights_data(loffset + 20).xy;
						float signed_atlas_size = lights_data(loffset + 20).z;
						float atlas_size  = abs(signed_atlas_size);
						float shadow_bias = light_params.z;
						float normal_bias = light_params.w;
						float shadow_texel_scale = lights_data(loffset + 20).w;
						float atlas_half_y = signed_atlas_size*0.5;
						float atlas_lower_y = min(atlas_half_y, 0.0);

						vec3 l = normalize(position - world_pos);
						float receiver_distance = length(world_pos - position);
						float world_texel_size = receiver_distance * shadow_texel_scale;
						vec3 biased_world_pos = shadow_bias_position(world_pos
							, geometric_n
							, l
							, world_texel_size
							, normal_bias
							);

						vec3 sl = biased_world_pos - position; // Transform to light-local space.

						// Select tetrahedron face.
						float b = dot(sl, bn);
						float y = dot(sl, yn);
						float g = dot(sl, gn);
						float r = dot(sl, rn);
						float maximum = max(max(b, y), max(g, r));

						mat4 shadow_mtx;
						vec3 atlas_offset;

						if (maximum == b) {
							// Tetrahedron mvp matrices.
							mat4 bmtx = mtxFromCols(lights_data(loffset + 4)
								, lights_data(loffset + 5)
								, lights_data(loffset + 6)
								, lights_data(loffset + 7)
								);
							shadow_mtx = bmtx;
							atlas_offset = vec3(atlas_base, atlas_size);
						} else if (maximum == y) {
							mat4 ymtx = mtxFromCols(lights_data(loffset +  8)
								, lights_data(loffset +  9)
								, lights_data(loffset + 10)
								, lights_data(loffset + 11)
								);
							shadow_mtx = ymtx;
							atlas_offset = vec3(atlas_base + vec2(0.0, atlas_half_y), atlas_size);
						} else if (maximum == g) {
							mat4 gmtx = mtxFromCols(lights_data(loffset + 12)
								, lights_data(loffset + 13)
								, lights_data(loffset + 14)
								, lights_data(loffset + 15)
								);
							shadow_mtx = gmtx;
							atlas_offset = vec3(atlas_base + vec2(0.0, atlas_lower_y), atlas_size);
						} else {
							mat4 rmtx = mtxFromCols(lights_data(loffset + 16)
								, lights_data(loffset + 17)
								, lights_data(loffset + 18)
								, lights_data(loffset + 19)
								);
							shadow_mtx = rmtx;
							atlas_offset = vec3(atlas_base + vec2(atlas_size*0.5, atlas_lower_y), atlas_size);
						}

						vec4 shadow_pos0 = mul(shadow_mtx, vec4(biased_world_pos, 1.0));
						vec4 atlas_shadow_pos0 = atlas_shadow_coord(shadow_pos0, atlas_offset);
						if (shadow_coord_inside_atlas_tile(atlas_shadow_pos0, atlas_offset)) {
							local_radiance *= shadow(u_local_lights_shadow_map
								, atlas_shadow_pos0
								, shadow_bias
								, local_lights_sm_texel_size
								, local_lights_shadow_map_samples
								);
						} else {
							local_radiance *= 0.0;
						}
					}

					if (has_cookie) {
						vec4 cookie_transform = lights_data(loffset + 21);
						vec4 cookie_up = lights_data(loffset + 22);
						vec4 cookie_rect = lights_data(loffset + 23);
						vec3 to_frag = normalize(world_pos - position);
						vec3 cookie_right = cross(direction, cookie_up.xyz);
						vec3 cookie_direction = vec3(dot(to_frag, cookie_right)
							, dot(to_frag, cookie_up.xyz)
							, dot(to_frag, direction)
							);
						vec2 cookie_uv = equirectangular_uv(cookie_direction);
						local_radiance *= sample_light_cookie(cookie_rect, cookie_uv, cookie_transform, 0.5, cookie_up.w);
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
					vec4 light_params = lights_data(loffset + 3);
					bool cast_shadow  = light_params.x == 1.0;
					bool has_cookie   = light_params.y == 1.0;

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

					bool receive_light_shadow = receive_shadow && cast_shadow;
					if (has_cookie || receive_light_shadow) {
						mat4 mvp = mtxFromCols(lights_data(loffset + 4)
							, lights_data(loffset + 5)
							, lights_data(loffset + 6)
							, lights_data(loffset + 7)
							);
						vec4 light_clip = mul(mvp, vec4(world_pos, 1.0));

						if (receive_light_shadow) {
							vec2 atlas_offset = lights_data(loffset + 20).xy;
							float atlas_size  = lights_data(loffset + 20).z;
							float shadow_bias = light_params.z;
							float normal_bias = light_params.w;
							float shadow_texel_scale = lights_data(loffset + 20).w;
							vec3 shadow_atlas_offset = vec3(atlas_offset, atlas_size);
							vec4 atlas_shadow_pos0 = atlas_shadow_coord(light_clip, shadow_atlas_offset);

							if (shadow_coord_inside_atlas_tile(atlas_shadow_pos0, shadow_atlas_offset)) {
								vec3 l = normalize(position - world_pos);
								float receiver_depth = abs(light_clip.w);
								float world_texel_size = receiver_depth * shadow_texel_scale;
								vec3 biased_world_pos = shadow_bias_position(world_pos
									, geometric_n
									, l
									, world_texel_size
									, normal_bias
									);
								vec4 shadow_clip = mul(mvp, vec4(biased_world_pos, 1.0));
								local_radiance *= shadow(u_local_lights_shadow_map
									, atlas_shadow_coord(shadow_clip, shadow_atlas_offset)
									, shadow_bias
									, local_lights_sm_texel_size
									, local_lights_shadow_map_samples
									);
							} else {
								local_radiance *= 0.0;
							}
						}

						if (has_cookie) {
							vec4 cookie_transform = lights_data(loffset + 21);
							vec4 cookie_rect = lights_data(loffset + 23);
							vec2 cookie_uv = light_clip.xy / light_clip.w;
							local_radiance *= sample_light_cookie(cookie_rect, cookie_uv, cookie_transform, 0.5, 0.0);
						}
					}

					radiance += apply_distance_fading(local_radiance, position, camera_pos);
				}

				return emission + radiance;
			}
		#endif // NO_LIGHT
		"""
	}
}
