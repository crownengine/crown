/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "resource/mesh_skeleton_resource.h"
#include "resource/mesh_animation_resource.h"
#include "world/types.h" // UnitId

namespace crown
{
struct AnimationTrackSegment
{
	AnimationKey keys[2];
};

struct MeshAnimation
{
	AnimationId id;
	u32 first_track_block;
	u32 num_tracks;
	const u16 *events_playhead;
	const AnimationKey *playhead; ///< Next key to fetch in the animation stream.
	const MeshAnimationResource *animation_resource;
};

#define MAX_ANIMATIONS       1024
#define ANIMATION_INDEX_MASK (MAX_ANIMATIONS - 1)
#define ANIMATION_ID_ADD     MAX_ANIMATIONS
#define MESH_ANIMATION_TRACKS_PER_BLOCK 32

struct MeshAnimationTrackBlock
{
	AnimationTrackSegment tracks[MESH_ANIMATION_TRACKS_PER_BLOCK];
	u32 next;
};

/// Evaluates bones' positions and rotations
/// in a mesh animation at a particular time.
struct MeshAnimationPlayer
{
	struct Index
	{
		AnimationId id;
		u32 index;      ///< Index into _animations.
		u32 next;       ///< Next free index slot.
	};

	u32 _freelist_dequeue;
	u32 _freelist_enqueue;
	Index _indices[MAX_ANIMATIONS];
	Array<MeshAnimation> _animations;
	Array<MeshAnimationTrackBlock> _tracks; ///< Blocks of currently evaluated animation track segments.
	u32 _free_track_block;

	///
	MeshAnimationPlayer(Allocator &a);
};

namespace mesh_animation_player
{
	///
	AnimationId create(MeshAnimationPlayer &p, const MeshAnimationResource *animation_resource);

	///
	void destroy(MeshAnimationPlayer &p, AnimationId anim_id);

	///
	bool has(MeshAnimationPlayer &p, AnimationId anim_id);

	///
	void evaluate(MeshAnimationPlayer &p, AnimationId anim_id, f32 time, UnitId unit, SceneGraph &scene_graph, const UnitId *bone_lookup, EventStream &events, bool reset);

	///
	void reload(MeshAnimationPlayer &p, const MeshAnimationResource *old_resource, const MeshAnimationResource *new_resource);

} // namespace mesh_animation_player

} // namespace crown
