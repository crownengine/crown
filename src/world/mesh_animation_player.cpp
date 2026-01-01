/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/math/constants.h"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/profiler.h"
#include "core/strings/string_id.inl"
#include "device/device.h"
#include "resource/resource_manager.h"
#include "world/mesh_animation_player.h"
#include "world/scene_graph.h"

namespace crown
{
namespace mesh_animation_player
{
	/// Copies @a num animation keys from @a playhead into the
	/// corresponding track segment @a segments. Returns a
	/// pointer to the new playhead.
	static const AnimationKey *fetch_keys(AnimationTrackSegment *tracks, const AnimationKey *playhead, u32 num = 1, u32 expected_track_id = UINT32_MAX)
	{
		if (expected_track_id != UINT32_MAX) {
			CE_ASSERT(expected_track_id == playhead->h.track_id
				, "Expected track %u stream gave %u"
				, expected_track_id
				, playhead->h.track_id
				);
		}

		AnimationTrackSegment *t = &tracks[playhead->h.track_id];
		for (u32 i = 0; i < num; ++i) {
			t->keys[0] = t->keys[1];
			t->keys[1] = *playhead++;
		}

		return playhead;
	}

	static void init(MeshAnimationPlayer &p, MeshAnimation &anim)
	{
		// Initialize tracks with animation data.
		// We need 2 samples for each animation track to begin interpolating curves.
		AnimationTrackSegment *tracks = array::begin(p._tracks) + anim.tracks_offset;
		for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
			anim.playhead = fetch_keys(tracks, anim.playhead, 1, track_id);
			anim.playhead = fetch_keys(tracks, anim.playhead, 1, track_id);
		}
	}

	AnimationId create(MeshAnimationPlayer &p, const MeshAnimationResource *animation_resource)
	{
		MeshAnimationPlayer::Index &index = p._indices[p._freelist_dequeue];
		p._freelist_dequeue = index.next;
		index.id += ANIMATION_ID_ADD;
		index.index = array::size(p._animations);

		MeshAnimation anim;
		anim.id = index.id;
		anim.tracks_offset = array::size(p._tracks);
		anim.num_tracks = animation_resource->num_tracks;
		anim.playhead = mesh_animation_resource::animation_keys(animation_resource);
		anim.animation_resource = animation_resource;
		// Allocate tracks.
		array::reserve(p._tracks, array::size(p._tracks) + animation_resource->num_tracks);
		p._tracks._size += animation_resource->num_tracks;
		init(p, anim);

		array::push_back(p._animations, anim);
		return anim.id;
	}

	void destroy(MeshAnimationPlayer &p, AnimationId anim_id)
	{
		MeshAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];

		MeshAnimation &a = p._animations[index.index];
		a = p._animations[array::size(p._animations) - 1];
		array::pop_back(p._animations);
		p._indices[a.id & ANIMATION_INDEX_MASK].index = index.index;

		index.index = UINT32_MAX;
		p._indices[p._freelist_enqueue].next = anim_id & ANIMATION_INDEX_MASK;
		p._freelist_enqueue = anim_id & ANIMATION_INDEX_MASK;
	}

	bool has(MeshAnimationPlayer &p, AnimationId anim_id)
	{
		MeshAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];
		return index.index != UINT32_MAX && index.id == anim_id;
	}

	void evaluate(MeshAnimationPlayer &p, AnimationId anim_id, f32 time, SceneGraph &scene_graph, const UnitId *bone_lookup)
	{
		MeshAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];
		MeshAnimation &anim = p._animations[index.index];
		AnimationTrackSegment *tracks = array::begin(p._tracks) + anim.tracks_offset;

		CE_ENSURE(time <= anim.animation_resource->total_time);
		u16 ts = time * 1000.0f;

		// Fetch new keys until all tracks have enough data
		// to interpolate values at current time.
		u32 num_ok = 0;
		while (num_ok != anim.num_tracks) {
			const AnimationKey *first_key = mesh_animation_resource::animation_keys(anim.animation_resource);
			if (anim.playhead - first_key == anim.animation_resource->num_keys) {
				anim.playhead = first_key;
				init(p, anim);
			}

			num_ok = 0;
			for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
				AnimationTrackSegment *track = &tracks[track_id];

				if (track->keys[0].h.time > ts || ts > track->keys[1].h.time)
					anim.playhead = fetch_keys(tracks, anim.playhead, 1, track_id);

				if (track->keys[0].h.time <= ts && ts <= track->keys[1].h.time)
					num_ok++;
			}
		}

		const u16 *bone_ids = mesh_animation_resource::bone_ids(anim.animation_resource);

		// Evaluate animation data at current time.
		for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
			AnimationTrackSegment *track = &tracks[track_id];

			CE_ENSURE(track->keys[0].h.time <= ts && ts <= track->keys[1].h.time);
			u16 n = ts - track->keys[0].h.time;
			u16 d = track->keys[1].h.time - track->keys[0].h.time;
			f32 t = f32(n)/f32(d);
			CE_ENSURE(t >= 0 && t <= 1);

			if (track->keys[0].h.type == AnimationKeyHeader::Type::POSITION) {
				Vector3 pos = lerp(track->keys[0].p.value, track->keys[1].p.value, t);
				TransformInstance ti = scene_graph.instance(bone_lookup[bone_ids[track_id]]);
				scene_graph.set_local_position(ti, pos);
			} else if (track->keys[0].h.type == AnimationKeyHeader::Type::ROTATION) {
				Quaternion rot = lerp(track->keys[0].r.value, track->keys[1].r.value, t);
				TransformInstance ti = scene_graph.instance(bone_lookup[bone_ids[track_id]]);
				scene_graph.set_local_rotation(ti, rot);
			} else {
				CE_FATAL("Unknown key type %u in track %u", track->keys[0].h.type, track_id);
			}
		}
	}

} // namespace mesh_animation_player

MeshAnimationPlayer::MeshAnimationPlayer(Allocator &a)
	: _animations(a)
	, _tracks(a)
{
	for (u32 i = 0; i < countof(_indices); ++i) {
		_indices[i].id = i;
		_indices[i].next = i + 1;
		_indices[i].index = UINT32_MAX;
	}

	_freelist_dequeue = 0;
	_freelist_enqueue = countof(_indices) - 1;
}

} // namespace crown
