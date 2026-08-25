/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "resource/mesh_animation_resource.h"

namespace crown
{
namespace mesh_animation_resource
{
	const AnimationKey *animation_keys(const MeshAnimationResource *mar)
	{
		return (AnimationKey *)((char *)mar + mar->keys_offset);
	}

	const u16 *bone_ids(const MeshAnimationResource *mar)
	{
		return (u16 *)((char *)mar + mar->bone_ids_offset);
	}

	const u16 *event_times(const MeshAnimationResource *mar)
	{
		return (u16 *)((char *)mar + mar->event_times_offset);
	}

	const StringId32 *event_names(const MeshAnimationResource *mar)
	{
		return (StringId32 *)((char *)mar + mar->event_names_offset);
	}

} // namespace mesh_animation_resource

namespace mesh_animation
{
	/// Interpolates a key pair into @a target, which must implement
	/// set_position(Vector3) and set_rotation(Quaternion).
	template<typename Target>
	inline void interpolate_track(Target &target
		, const AnimationKey &a
		, const AnimationKey &b
		, f32 t
		)
	{
		if (a.h.type == AnimationKeyHeader::Type::POSITION)
			target.set_position(lerp(a.p.value, b.p.value, t));
		else
			target.set_rotation(lerp(a.r.value, b.r.value, t));
	}

	/// Evaluates a key pair with distinct timestamps into @a target.
	template<typename Target>
	inline void evaluate_track(Target &target
		, const AnimationKey &a
		, const AnimationKey &b
		, u16 time
		)
	{
		const u16 elapsed = time - a.h.time;
		const u16 duration = b.h.time - a.h.time;
		const f32 t = f32(elapsed) / f32(duration);
		CE_ENSURE(t >= 0.0f && t <= 1.0f);
		interpolate_track(target, a, b, t);
	}

} // namespace mesh_animation

} // namespace crown
