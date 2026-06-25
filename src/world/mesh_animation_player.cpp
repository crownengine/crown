/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/event_stream.inl"
#include "core/math/constants.h"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/profiler.h"
#include "core/strings/string_id.inl"
#include "device/device.h"
#include "resource/mesh_animation_resource.inl"
#include "resource/resource_manager.h"
#include "world/mesh_animation_player.h"
#include "world/scene_graph.h"

namespace crown
{
namespace mesh_animation_player
{
	static u32 alloc_track_block(MeshAnimationPlayer &p)
	{
		if (p._free_track_block != UINT32_MAX) {
			const u32 block_index = p._free_track_block;
			p._free_track_block = p._tracks[block_index].next;
			p._tracks[block_index].next = UINT32_MAX;
			return block_index;
		}

		MeshAnimationTrackBlock block;
		block.next = UINT32_MAX;
		array::push_back(p._tracks, block);
		return array::size(p._tracks) - 1;
	}

	static u32 alloc_track_blocks(MeshAnimationPlayer &p, u32 num_tracks)
	{
		if (num_tracks == 0)
			return UINT32_MAX;

		const u32 num_blocks = (num_tracks + MESH_ANIMATION_TRACKS_PER_BLOCK - 1)/MESH_ANIMATION_TRACKS_PER_BLOCK;
		const u32 first_block = alloc_track_block(p);
		u32 prev_block = first_block;

		for (u32 i = 1; i < num_blocks; ++i) {
			const u32 block = alloc_track_block(p);
			p._tracks[prev_block].next = block;
			prev_block = block;
		}

		return first_block;
	}

	static void free_track_blocks(MeshAnimationPlayer &p, u32 first_block)
	{
		if (first_block == UINT32_MAX)
			return;

		u32 last_block = first_block;
		while (p._tracks[last_block].next != UINT32_MAX)
			last_block = p._tracks[last_block].next;

		p._tracks[last_block].next = p._free_track_block;
		p._free_track_block = first_block;
	}

	static AnimationTrackSegment *track_segment(MeshAnimationPlayer &p, MeshAnimation &anim, u32 track_id)
	{
		u32 block = anim.first_track_block;
		while (track_id >= MESH_ANIMATION_TRACKS_PER_BLOCK) {
			CE_ASSERT(block != UINT32_MAX, "Track block not found");
			block = p._tracks[block].next;
			track_id -= MESH_ANIMATION_TRACKS_PER_BLOCK;
		}

		CE_ASSERT(block != UINT32_MAX, "Track block not found");
		return &p._tracks[block].tracks[track_id];
	}

	/// Copies the animation key from @a playhead into the corresponding track segment. Returns a
	/// pointer to the new playhead.
	static const AnimationKey *fetch_key(MeshAnimationPlayer &p, MeshAnimation &anim, const AnimationKey *playhead)
	{
		AnimationTrackSegment *t = track_segment(p, anim, playhead->h.track_id);
		t->keys[0] = t->keys[1];
		t->keys[1] = *playhead++;

		return playhead;
	}

	static bool track_is_valid(AnimationTrackSegment *track, u16 ts)
	{
		return track->keys[0].h.time <= ts && ts <= track->keys[1].h.time;
	}

	static bool tracks_are_valid(MeshAnimationPlayer &p, MeshAnimation &anim, u16 ts)
	{
		for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
			AnimationTrackSegment *track = track_segment(p, anim, track_id);
			if (!track_is_valid(track, ts))
				return false;
		}

		return true;
	}

	static void init(MeshAnimationPlayer &p, MeshAnimation &anim)
	{
		// Initialize tracks with animation data.
		// We need 2 samples for each animation track to begin interpolating curves.
		for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
			CE_ASSERT(anim.playhead->h.track_id == track_id
				, "Expected track %u stream gave %u"
				, track_id
				, anim.playhead->h.track_id
				);
			anim.playhead = fetch_key(p, anim, anim.playhead);
			CE_ASSERT(anim.playhead->h.track_id == track_id
				, "Expected track %u stream gave %u"
				, track_id
				, anim.playhead->h.track_id
				);
			anim.playhead = fetch_key(p, anim, anim.playhead);
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
		anim.num_tracks = animation_resource->num_tracks;
		anim.first_track_block = alloc_track_blocks(p, anim.num_tracks);
		anim.events_playhead = mesh_animation_resource::event_times(animation_resource);
		anim.playhead = mesh_animation_resource::animation_keys(animation_resource);
		anim.animation_resource = animation_resource;
		init(p, anim);

		array::push_back(p._animations, anim);
		return anim.id;
	}

	void destroy(MeshAnimationPlayer &p, AnimationId anim_id)
	{
		MeshAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];

		MeshAnimation &a = p._animations[index.index];
		free_track_blocks(p, a.first_track_block);
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

	void evaluate(MeshAnimationPlayer &p, AnimationId anim_id, f32 time, UnitId unit, SceneGraph &scene_graph, const UnitId *bone_lookup, EventStream &events, bool reset)
	{
		MeshAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];
		MeshAnimation &anim = p._animations[index.index];

		CE_ENSURE(time <= anim.animation_resource->total_time);
		u16 ts = u16(time * 1000.0f);

		// Fetch new keys until all tracks have enough data to interpolate
		// values at current time. Keys must be consumed in stream order.
		const AnimationKey *first_key = mesh_animation_resource::animation_keys(anim.animation_resource);
		const AnimationKey *end_key = first_key + anim.animation_resource->num_keys;
		for (;;) {
			if (anim.playhead == end_key) {
				if (tracks_are_valid(p, anim, ts))
					break;

				anim.playhead = first_key;
				init(p, anim);
			}

			AnimationTrackSegment *track = track_segment(p, anim, anim.playhead->h.track_id);
			if (track_is_valid(track, ts))
				break;

			anim.playhead = fetch_key(p, anim, anim.playhead);
		}

		const u16 *bone_ids = mesh_animation_resource::bone_ids(anim.animation_resource);

		// Evaluate animation data at current time.
		for (u32 track_id = 0; track_id < anim.num_tracks; ++track_id) {
			AnimationTrackSegment *track = track_segment(p, anim, track_id);

			CE_ENSURE(track->keys[0].h.time <= ts && ts <= track->keys[1].h.time);
			u16 n = ts - track->keys[0].h.time;
			u16 d = track->keys[1].h.time - track->keys[0].h.time;
			f32 t = f32(n)/f32(d);
			CE_ENSURE(t >= 0 && t <= 1);

			if (track->keys[0].h.type == AnimationKeyHeader::Type::POSITION) {
				Vector3 pos = lerp(track->keys[0].p.value, track->keys[1].p.value, t);
				TransformId ti = scene_graph.instance(bone_lookup[bone_ids[track_id]]);
				scene_graph.set_local_position(ti, pos);
			} else if (track->keys[0].h.type == AnimationKeyHeader::Type::ROTATION) {
				Quaternion rot = lerp(track->keys[0].r.value, track->keys[1].r.value, t);
				TransformId ti = scene_graph.instance(bone_lookup[bone_ids[track_id]]);
				scene_graph.set_local_rotation(ti, rot);
			} else {
				CE_FATAL("Unknown key type %u in track %u", track->keys[0].h.type, track_id);
			}
		}

		// Generate events.
		const u16 *event_times = mesh_animation_resource::event_times(anim.animation_resource);
		const u16 *event_end = event_times + anim.animation_resource->num_events;
		const StringId32 *event_names = mesh_animation_resource::event_names(anim.animation_resource);
		while (anim.events_playhead != event_end && *anim.events_playhead <= ts) {
			UnitEvent ev;
			ev.unit = unit;
			ev.name = event_names[anim.events_playhead - event_times];
			event_stream::write(events, 1, ev);

			++anim.events_playhead;
		}

		if (reset)
			anim.events_playhead = mesh_animation_resource::event_times(anim.animation_resource);
	}

	void reload(MeshAnimationPlayer &p, const MeshAnimationResource *old_resource, const MeshAnimationResource *new_resource)
	{
		for (u32 i = 0; i < array::size(p._animations); ++i) {
			MeshAnimation &anim = p._animations[i];

			if (anim.animation_resource == old_resource) {
				free_track_blocks(p, anim.first_track_block);
				anim.num_tracks = new_resource->num_tracks;
				anim.first_track_block = alloc_track_blocks(p, anim.num_tracks);
				anim.events_playhead = mesh_animation_resource::event_times(new_resource);
				anim.playhead = mesh_animation_resource::animation_keys(new_resource);
				anim.animation_resource = new_resource;
				init(p, anim);
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
	_free_track_block = UINT32_MAX;
}

} // namespace crown
