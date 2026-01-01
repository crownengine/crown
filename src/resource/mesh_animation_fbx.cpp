/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_animation_resource.h"

#if CROWN_CAN_COMPILE
#   include "core/memory/globals.h"
#   include "core/strings/dynamic_string.inl"
#   include "resource/compile_options.inl"
#   include "resource/fbx_document.h"
#   include "resource/mesh_animation.h"
#   include "resource/mesh_skeleton.h"
#   include <ufbx.h>

namespace crown
{
namespace fbx
{
	static s32 parse_animation(MeshAnimation &ma, FBXDocument &fbx, ufbx_anim *anim, CompileOptions &opts)
	{
		ufbx_error bake_error;
		ufbx_baked_anim *bake = ufbx_bake_anim(fbx.scene, anim, NULL, &bake_error);
		RETURN_IF_FALSE(fbx.scene != NULL
			, opts
			, "ufbx: %s"
			, bake_error.description.data
			);

		ma.total_time = bake->playback_duration;

		for (size_t i = 0; i < bake->nodes.count; i++) {
			ufbx_baked_node *bake_node = &bake->nodes.data[i];
			ufbx_node *scene_node = fbx.scene->nodes.data[bake_node->typed_id];

			u16 bone_id = fbx::bone_id(fbx, scene_node->name.data);
			RETURN_IF_FALSE(bone_id != UINT16_MAX
				, opts
				, "Bone '%s' not found in FBX source"
				, scene_node->name.data
				);
			RETURN_IF_FALSE(bone_id < MESH_SKELETON_MAX_BONES
				, opts
				, "Maximum number of bones reached %u"
				, MESH_SKELETON_MAX_BONES
				);

			AnimationKeyIndex ki;
			ki.h.type = AnimationKeyHeader::POSITION;
			ki.h.track_id = mesh_animation::track_id(ma, bone_id, ki.h.type);
			ki.offset = array::size(ma.keys);
			ki.num = max(2u, (u32)bake_node->translation_keys.count);
			ki.cur = 0;
			array::push_back(ma.indices, ki);

			for (size_t j = 0; j < bake_node->translation_keys.count; ++j) {
				ufbx_baked_vec3 *bake_vec3 = &bake_node->translation_keys.data[j];

				AnimationKey key;
				key.h.type = AnimationKeyHeader::POSITION;
				key.h.track_id = mesh_animation::track_id(ma, bone_id, key.h.type);
				key.h.time = u16(bake_vec3->time * 1000.0f);
				key.p.value.x = (f32)bake_vec3->value.x;
				key.p.value.y = (f32)bake_vec3->value.y;
				key.p.value.z = (f32)bake_vec3->value.z;
				array::push_back(ma.keys, key);
			}

			if (bake_node->translation_keys.count == 1) {
				AnimationKey end_key = array::back(ma.keys);
				end_key.h.time = bake->playback_duration * 1000.0f;
				array::push_back(ma.keys, end_key);
			}

			ki.h.type = AnimationKeyHeader::ROTATION;
			ki.h.track_id = mesh_animation::track_id(ma, bone_id, ki.h.type);
			ki.offset = array::size(ma.keys);
			ki.num = max(2u, (u32)bake_node->rotation_keys.count);
			ki.cur = 0;
			array::push_back(ma.indices, ki);

			for (size_t j = 0; j < bake_node->rotation_keys.count; ++j) {
				ufbx_baked_quat *bake_quat = &bake_node->rotation_keys.data[j];

				AnimationKey key;
				key.h.type = AnimationKeyHeader::ROTATION;
				key.h.track_id = mesh_animation::track_id(ma, bone_id, key.h.type);
				key.h.time = u16(bake_quat->time * 1000.0f);
				key.r.value.x = (f32)bake_quat->value.x;
				key.r.value.y = (f32)bake_quat->value.y;
				key.r.value.z = (f32)bake_quat->value.z;
				key.r.value.w = (f32)bake_quat->value.w;
				array::push_back(ma.keys, key);
			}

			if (bake_node->rotation_keys.count == 1) {
				AnimationKey end_key = array::back(ma.keys);
				end_key.h.time = bake->playback_duration * 1000.0f;
				array::push_back(ma.keys, end_key);
			}
		}

		ufbx_free_baked_anim(bake);
		return 0;
	}

	static s32 parse_animations(MeshAnimation &ma, FBXDocument &fbx, CompileOptions &opts)
	{
		// Find matching animation in FBX file.
		for (size_t i = 0; i < fbx.scene->anim_stacks.count; ++i) {
			ufbx_anim_stack *stack = fbx.scene->anim_stacks.data[i];
			if (ma.stack_name == stack->name.data)
				return parse_animation(ma, fbx, stack->anim, opts);
		}

		opts.error("No matching animation '%s' in FBX source", ma.stack_name.c_str());
		return -1;
	}

	s32 parse(MeshAnimation &ma, Buffer &buf, CompileOptions &opts)
	{
		FBXDocument fbx(default_allocator());
		s32 err = fbx::parse(fbx, buf, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		return parse_animations(ma, fbx, opts);
	}

} // namespace fbx

} // namespace crown
#endif // if CROWN_CAN_COMPILE
