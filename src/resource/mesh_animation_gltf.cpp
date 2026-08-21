/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_animation_gltf.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/vector.inl"
#   include "core/math/constants.h"
#   include "core/math/math.h"
#   include "core/math/matrix4x4.inl"
#   include "core/math/quaternion.inl"
#   include "core/memory/globals.h"
#   include "core/strings/dynamic_string.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/gltf_document.h"
#   include "resource/mesh_skeleton_resource.h"
#   include <cgltf.h>

LOG_SYSTEM(MESH_ANIMATION_GLTF, "mesh_animation_gltf")

namespace crown
{
namespace gltf
{
	static bool read_value(const cgltf_accessor *accessor, cgltf_size index, f32 *value, cgltf_size count)
	{
		return index < accessor->count
			&& cgltf_accessor_read_float(accessor, index, value, count)
			;
	}

	static s32 sample_channel(f32 *out
		, const cgltf_animation_channel &channel
		, f32 time
		, cgltf_size components
		, CompileOptions &opts
		)
	{
		const cgltf_animation_sampler &sampler = *channel.sampler;
		const cgltf_accessor *input = sampler.input;
		const cgltf_accessor *output = sampler.output;
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, input != NULL && output != NULL && input->count != 0, opts
			, "glTF animation channel has invalid accessors"
			);

		f32 first_time;
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(input, 0, &first_time, 1), opts
			, "Failed to read glTF animation time"
			);
		if (input->count == 1 || time <= first_time) {
			const cgltf_size index = sampler.interpolation == cgltf_interpolation_type_cubic_spline ? 1 : 0;
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(output, index, out, components), opts
				, "Failed to read glTF animation value"
				);
			return 0;
		}

		cgltf_size right = 1;
		f32 right_time = first_time;
		for (; right < input->count; ++right) {
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(input, right, &right_time, 1), opts
				, "Failed to read glTF animation time"
				);
			if (time <= right_time)
				break;
		}
		if (right == input->count) {
			right = input->count - 1;
			const cgltf_size index = sampler.interpolation == cgltf_interpolation_type_cubic_spline
				? right*3 + 1
				: right
				;
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(output, index, out, components), opts
				, "Failed to read glTF animation value"
				);
			return 0;
		}

		const cgltf_size left = right - 1;
		f32 left_time;
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(input, left, &left_time, 1), opts
			, "Failed to read glTF animation time"
			);
		const f32 duration = right_time - left_time;
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, duration > 0.0f, opts
			, "glTF animation input times must be strictly increasing"
			);
		const f32 t = clamp((time - left_time) / duration, 0.0f, 1.0f);

		f32 a[4] = {};
		f32 b[4] = {};
		if (sampler.interpolation == cgltf_interpolation_type_step) {
			const cgltf_size step = time >= right_time ? right : left;
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, read_value(output, step, out, components), opts
				, "Failed to read glTF animation value"
				);
		} else if (sampler.interpolation == cgltf_interpolation_type_linear) {
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF
				, read_value(output, left, a, components) && read_value(output, right, b, components)
				, opts
				, "Failed to read glTF animation values"
				);
			if (components == 4) {
				Quaternion q = crown::slerp({ a[0], a[1], a[2], a[3] }, { b[0], b[1], b[2], b[3] }, t);
				out[0] = q.x; out[1] = q.y; out[2] = q.z; out[3] = q.w;
			} else {
				for (cgltf_size i = 0; i < components; ++i)
					out[i] = a[i] + (b[i] - a[i])*t;
			}
		} else {
			f32 out_tangent[4] = {};
			f32 in_tangent[4] = {};
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF
				, read_value(output, left*3 + 1, a, components)
				&& read_value(output, left*3 + 2, out_tangent, components)
				&& read_value(output, right*3 + 0, in_tangent, components)
				&& read_value(output, right*3 + 1, b, components)
				, opts
				, "Failed to read glTF cubic-spline animation values"
				);
			for (cgltf_size i = 0; i < components; ++i)
				out[i] = hermite(a[i], duration*out_tangent[i], b[i], duration*in_tangent[i], t);
			if (components == 4) {
				Quaternion q = { out[0], out[1], out[2], out[3] };
				normalize(q);
				out[0] = q.x; out[1] = q.y; out[2] = q.z; out[3] = q.w;
			}
		}

		return 0;
	}

	static s32 animation_range(f32 &start, f32 &duration, const cgltf_animation &animation, CompileOptions &opts)
	{
		start = 0.0f;
		f32 end = 0.0f;
		for (cgltf_size i = 0; i < animation.samplers_count; ++i) {
			const cgltf_accessor *input = animation.samplers[i].input;
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, input != NULL && input->count != 0, opts
				, "glTF animation sampler has no input keys"
				);
			f32 first;
			f32 last;
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF
				, read_value(input, 0, &first, 1)
				&& read_value(input, input->count - 1, &last, 1)
				, opts
				, "Failed to read glTF animation range"
				);
			RETURN_IF_FALSE(MESH_ANIMATION_GLTF, last >= first, opts
				, "glTF animation input times must be increasing"
				);
			if (i == 0) {
				start = first;
				end = last;
			} else {
				start = min(start, first);
				end = max(end, last);
			}
		}
		duration = end - start;
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, duration * 1000.0f <= (f32)UINT16_MAX, opts
			, "glTF animation is too long: %.3f seconds (maximum %.3f)"
			, duration
			, (f32)UINT16_MAX / 1000.0f
			);
		return 0;
	}

	static bool skin_contains_node(const cgltf_skin &skin, const cgltf_node *node)
	{
		for (cgltf_size i = 0; i < skin.joints_count; ++i) {
			for (const cgltf_node *joint = skin.joints[i]; joint != NULL; joint = joint->parent) {
				if (joint == node)
					return true;
			}
		}
		return false;
	}

	static bool node_in_other_skin_group(const GLTFDocument &doc, const cgltf_node *node)
	{
		const u32 selected_group = skin_group(doc, doc.skin);
		for (cgltf_size i = 0; i < doc.data->skins_count; ++i) {
			const cgltf_skin *skin = &doc.data->skins[i];
			if (skin_group(doc, skin) != selected_group && skin_contains_node(*skin, node))
				return true;
		}
		return false;
	}

	static s32 bake_channel(MeshAnimation &ma
		, const GLTFDocument &doc
		, const cgltf_animation_channel &channel
		, const Matrix4x4 &root_transform
		, f32 animation_start
		, u32 num_samples
		, CompileOptions &opts
		)
	{
		if (channel.target_path == cgltf_animation_path_type_scale
			|| channel.target_path == cgltf_animation_path_type_weights
			) {
			opts.warning(MESH_ANIMATION_GLTF, "glTF scale and morph animations are not supported and will be ignored");
			return 0;
		}

		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, channel.target_node != NULL, opts
			, "glTF animation channel has no target node"
			);
		const u16 target_bone = bone_id(doc, channel.target_node);
		if (target_bone == UINT16_MAX) {
			if (!node_in_other_skin_group(doc, channel.target_node)) {
				opts.warning(MESH_ANIMATION_GLTF, "glTF animation target '%s' is not in the selected skeleton and will be ignored"
					, node_name(doc, channel.target_node)
					);
			}
			return 0;
		}

		const u16 type = channel.target_path == cgltf_animation_path_type_translation
			? AnimationKeyHeader::POSITION
			: AnimationKeyHeader::ROTATION
			;
		AnimationKeyIndex index;
		index.h.type = type;
		index.h.track_id = mesh_animation::track_id(ma, target_bone, type);
		index.offset = array::size(ma.keys);
		index.num = num_samples;
		index.cur = 0;
		array::push_back(ma.indices, index);
		const bool root_bone = channel.target_node->parent == NULL
			|| bone_id(doc, channel.target_node->parent) == UINT16_MAX
			;

		for (u32 sample = 0; sample < num_samples; ++sample) {
			const f32 time = sample == num_samples - 1
				? ma.total_time
				: (f32)sample / 30.0f
				;
			f32 value[4] = {};
			s32 err = sample_channel(value, channel, animation_start + time
				, type == AnimationKeyHeader::POSITION ? 3 : 4
				, opts
				);
			ENSURE_OR_RETURN(MESH_ANIMATION_GLTF, err == 0, opts);

			AnimationKey key;
			key.h.type = type;
			key.h.track_id = index.h.track_id;
			key.h.time = (u16)fround(time * 1000.0f);
			if (type == AnimationKeyHeader::POSITION) {
				key.p.value = vector3(value);
				if (root_bone)
					key.p.value = key.p.value * root_transform;
			} else {
				key.r.value = quaternion(value);
				if (root_bone) {
					Matrix4x4 pose = from_quaternion_translation(key.r.value, VECTOR3_ZERO);
					key.r.value = rotation(pose * root_transform);
				}
			}
			array::push_back(ma.keys, key);
		}

		return 0;
	}

	s32 parse(MeshAnimation &ma, const char *path, const char *skin_name, CompileOptions &opts)
	{
		GLTFDocument doc(default_allocator());
		s32 err = gltf::parse(doc, path, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_GLTF, err == 0, opts);
		err = select_skin(doc, skin_name, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_GLTF, err == 0, opts);
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, vector::size(doc.bones) != 0, opts
			, "No animated skeleton in glTF source"
			);

		cgltf_animation *animation = NULL;
		for (cgltf_size i = 0; i < doc.data->animations_count; ++i) {
			if (ma.stack_name == animation_name(doc, &doc.data->animations[i])) {
				animation = &doc.data->animations[i];
				break;
			}
		}
		RETURN_IF_FALSE(MESH_ANIMATION_GLTF, animation != NULL, opts
			, "No matching animation '%s' in glTF source", ma.stack_name.c_str()
			);

		f32 animation_start;
		err = animation_range(animation_start, ma.total_time, *animation, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_GLTF, err == 0, opts);
		// Encode zero-duration glTF poses as 1 ms clips.
		ma.total_time = max(ma.total_time, 1.0f / 1000.0f);
		const u32 num_samples = max(2u, (u32)fceil(ma.total_time * 30.0f) + 1u);
		const Matrix4x4 root_transform = skeleton_root_transform(doc);
		for (cgltf_size i = 0; i < animation->channels_count; ++i) {
			err = bake_channel(ma, doc, animation->channels[i], root_transform
				, animation_start
				, num_samples
				, opts
				);
			ENSURE_OR_RETURN(MESH_ANIMATION_GLTF, err == 0, opts);
		}
		return 0;
	}

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
