/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_animation.h"

#if CROWN_CAN_COMPILE
#   include "core/error/error.inl"
#   include "core/json/json_object.inl"
#   include "core/json/sjson.h"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "core/strings/string_id.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/mesh_animation_fbx.h"
#   include "resource/mesh_skeleton.h"
#   include <algorithm> // std::sort

#define DUMP_KEYS 0

LOG_SYSTEM(MESH_ANIMATION, "mesh_animation")

namespace crown
{
namespace mesh_animation
{
#if DUMP_KEYS
	static void dump_keys(AnimationKey *begin, AnimationKey *end)
	{
		char buf[256];

		for (auto cur = begin; cur != end; ++cur)
			logi(MESH_ANIMATION, "b %hu t %hu type %hu val %s"
				, cur->h.track_id
				, cur->h.time
				, cur->h.type
				, cur->h.type == 0
				? to_string(buf, sizeof(buf), cur->p.value)
				: to_string(buf, sizeof(buf), cur->r.value)
				);
	}
#endif

	u16 track_id(MeshAnimation &a, u16 bone_id, u16 parameter_type)
	{
		CE_ENSURE(bone_id < MESH_SKELETON_MAX_BONES);
		CE_ENSURE(parameter_type < AnimationKeyHeader::Type::COUNT);

		u16 t = (bone_id << 2) | u16(parameter_type);

		u16 track_id_not_found = UINT16_MAX;
		u16 track_id = hash_map::get(a.track_ids, t, track_id_not_found);
		if (track_id == track_id_not_found) {
			track_id = array::size(a.bone_ids);
			array::push_back(a.bone_ids, bone_id);
			hash_map::set(a.track_ids, t, track_id);
		}

		return track_id;
	}

	static s32 generate_sorted_keys(MeshAnimation &ma)
	{
#if 0
		// Test data.
		array::clear(ma.keys);
		array::clear(ma.indices);

		array::push_back(ma.indices, { { 0, 0, 0 }, array::size(ma.keys), 2 });
		array::push_back(ma.keys, { { 0, 0,  0 } });
		array::push_back(ma.keys, { { 0, 0, 10 } });

		array::push_back(ma.indices, { { 0, 1, 0 }, array::size(ma.keys), 4 });
		array::push_back(ma.keys, { { 0, 1,  0 } });
		array::push_back(ma.keys, { { 0, 1,  1 } });
		array::push_back(ma.keys, { { 0, 1,  5 } });
		array::push_back(ma.keys, { { 0, 1, 10 } });

		array::push_back(ma.indices, { { 0, 2, 0 }, array::size(ma.keys), 4 });
		array::push_back(ma.keys, { { 0, 2,  0 } });
		array::push_back(ma.keys, { { 0, 2,  6 } });
		array::push_back(ma.keys, { { 0, 2,  8 } });
		array::push_back(ma.keys, { { 0, 2, 10 } });
#endif // if 0

		// Sort indices by track ID. This ensures that when we encounter multiple keys
		// with matching times, we choose the key with the smallest track ID first.
		std::sort(array::begin(ma.indices)
			, array::end(ma.indices)
			, [](const AnimationKeyIndex &a, const AnimationKeyIndex &b) {
				return a.h.track_id < b.h.track_id;
			});

		// Generate a list of animation keys sorted by key access time.
		// Start by getting the first two keys for each track.
		for (u32 i = 0; i < array::size(ma.indices); ++i) {
			AnimationKeyIndex &idx = ma.indices[i];

			array::push_back(ma.sorted_keys, ma.keys[idx.offset + idx.cur++]);
			array::push_back(ma.sorted_keys, ma.keys[idx.offset + idx.cur++]);
		}

		while (array::size(ma.sorted_keys) != array::size(ma.keys)) {
			AnimationKeyIndex *next_key = NULL;
			// For each track, choose the key that will be needed next.
			for (u32 i = 0; i < array::size(ma.indices); ++i) {
				AnimationKeyIndex &idx = ma.indices[i];
				// There are no more keys in this track. Skip it.
				if (idx.cur > idx.num - 1)
					continue;

				// Select this as the next key if none have been selected so far.
				if (next_key == NULL) {
					next_key = &idx;
					continue;
				} else {
					// If next key's previous time is greater than current
					// key's previous time, then we need to get this key next.
					auto next_prev_time = ma.keys[next_key->offset + next_key->cur - 1].h.time;
					auto this_prev_time = ma.keys[idx.offset + idx.cur - 1].h.time;
					if (next_prev_time > this_prev_time)
						next_key = &idx;
				}
			}

			CE_ENSURE(next_key != NULL);
			array::push_back(ma.sorted_keys, ma.keys[next_key->offset + next_key->cur]);
			++next_key->cur;
		}

#if DUMP_KEYS
		dump_keys(array::begin(ma.sorted_keys), array::end(ma.sorted_keys));
#endif
		return 0;
	}

	s32 parse(MeshAnimation &ma, Buffer &buf, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		// Parse skeleton.
		DynamicString target_skeleton(ta);
		RETURN_IF_ERROR(sjson::parse_string(target_skeleton, obj["target_skeleton"]), opts);
		RETURN_IF_RESOURCE_MISSING("mesh_skeleton", target_skeleton.c_str(), opts);
		opts.add_requirement("mesh_skeleton", target_skeleton.c_str());
		ma.target_skeleton = RETURN_IF_ERROR(sjson::parse_resource_name(obj["target_skeleton"]), opts);

		// Parse animations.
		RETURN_IF_ERROR(sjson::parse_string(ma.stack_name, obj["stack_name"]), opts);

		DynamicString source(ta);
		if (json_object::has(obj, "source")) {
			RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]), opts);

			RETURN_IF_FILE_MISSING(source.c_str(), opts);
			Buffer fbx_buf = opts.read(source.c_str());
			s32 err = fbx::parse(ma, fbx_buf, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		} else {
			RETURN_IF_FALSE(false
				, opts
				, "Unknown source mesh '%s'"
				, source.c_str()
				);
		}

		return generate_sorted_keys(ma);
	}

} // namespace mesh_animation

MeshAnimation::MeshAnimation(Allocator &a)
	: sorted_keys(a)
	, keys(a)
	, indices(a)
	, num_bones(0u)
	, total_time(0.0f)
	, target_skeleton(u64(0u))
	, stack_name(a)
	, track_ids(a)
	, bone_ids(a)
{
}

} // namespace crown
#endif // if CROWN_CAN_COMPILE
