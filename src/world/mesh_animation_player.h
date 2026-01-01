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
	u32 tracks_offset;
	u32 num_tracks;
	const AnimationKey *playhead; ///< Next key to fetch in the animation stream.
	const MeshAnimationResource *animation_resource;
};

#define MAX_ANIMATIONS       1024
#define ANIMATION_INDEX_MASK (MAX_ANIMATIONS - 1)
#define ANIMATION_ID_ADD     MAX_ANIMATIONS

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
	Array<AnimationTrackSegment> _tracks; ///< The currently evaluated segment in each animation track.

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
	void evaluate(MeshAnimationPlayer &p, AnimationId anim_id, f32 time, SceneGraph &scene_graph, const UnitId *bone_lookup);

} // namespace mesh_animation_player

} // namespace crown
