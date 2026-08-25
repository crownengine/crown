/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "resource/mesh_animation_resource.inl"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/sphere.inl"
#include "core/math/vector3.inl"
#include "core/memory/globals.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh.h"
#include "resource/mesh_animation.h"
#include "resource/mesh_skeleton.h"
#include "resource/resource_id.inl"
#include <algorithm>

LOG_SYSTEM(MESH_ANIMATION_RESOURCE, "mesh_animation_resource")

namespace crown
{
namespace mesh_animation_resource_internal
{
	CE_STATIC_ASSERT(Geometry::MAX_BONE_WEIGHTS == 4);

	struct SampleBounds
	{
		AABB aabb;
		Sphere sphere;
	};

	struct BoundsAccumulator
	{
		bool valid;
		AABB aabb;
		Sphere sphere;
	};

	struct SampleContext
	{
		const MeshAnimation &_animation;
		const AnimationSkeleton &_skeleton;
		const Geometry &_geometry;
		Array<BoneTransform> _pose;
		Array<Matrix4x4> _world;
		Array<Matrix4x4> _palette;
		Array<Vector3> _points;

		SampleContext(Allocator &a
			, const MeshAnimation &animation
			, const AnimationSkeleton &skeleton
			, const Geometry &geometry
			)
			: _animation(animation)
			, _skeleton(skeleton)
			, _geometry(geometry)
			, _pose(a)
			, _world(a)
			, _palette(a)
			, _points(a)
		{
			array::resize(_pose, array::size(skeleton.local_transforms));
			array::resize(_world, array::size(skeleton.local_transforms));
			array::resize(_palette, array::size(skeleton.local_transforms));
			array::resize(_points, array::size(geometry._position_indices));
		}
	};

	struct BoneTransformTarget
	{
		BoneTransform &_bone;

		void set_position(const Vector3 &position)
		{
			_bone.position = position;
		}

		void set_rotation(const Quaternion &rotation)
		{
			_bone.rotation = rotation;
		}
	};

	static void add_sample(BoundsAccumulator &bounds, const SampleBounds &sample)
	{
		if (!bounds.valid) {
			bounds.aabb = sample.aabb;
			bounds.sphere = sample.sphere;
			bounds.valid = true;
		} else {
			bounds.aabb = aabb::merge(bounds.aabb, sample.aabb);
			bounds.sphere = sphere::merge(bounds.sphere, sample.sphere);
		}
	}

	static SampleBounds sample_bounds(SampleContext &ctx, u16 ts)
	{
		const MeshAnimation &animation = ctx._animation;
		const AnimationSkeleton &skeleton = ctx._skeleton;
		const Geometry &geometry = ctx._geometry;

		// A clip may omit tracks, so every independent sample starts from rest pose.
		for (u32 i = 0; i < array::size(ctx._pose); ++i)
			ctx._pose[i] = skeleton.local_transforms[i];

		// Evaluate tracks.
		for (u32 i = 0; i < array::size(animation.indices); ++i) {
			const AnimationKeyIndex &index = animation.indices[i];
			const AnimationKey *keys = array::begin(animation.keys) + index.offset;
			const AnimationKey *a = &keys[0];
			const AnimationKey *b = &keys[0];

			if (ts >= keys[index.num - 1].h.time) {
				a = b = &keys[index.num - 1];
			} else if (ts > keys[0].h.time) {
				u32 next = 1;
				while (keys[next].h.time < ts)
					++next;
				a = &keys[next - 1];
				b = &keys[next];
			}

			const u16 bone = animation.bone_ids[index.h.track_id];
			BoneTransformTarget target = { ctx._pose[bone] };
			if (a->h.time != b->h.time)
				mesh_animation::evaluate_track(target, *a, *b, ts);
			else
				mesh_animation::interpolate_track(target, *a, *b, 0.0f);
		}

		// Skin model.
		for (u32 i = 0; i < array::size(ctx._pose); ++i) {
			const BoneTransform &bone = ctx._pose[i];
			const Matrix4x4 local = mesh_animation::bone_local_transform(bone);

			const u32 parent = skeleton.parents[i];
			ctx._world[i] = parent == UINT16_MAX
				? local
				: local * ctx._world[parent]
				;
			ctx._palette[i] = i == 0
				? MATRIX4X4_IDENTITY
				: mesh_animation::skinning_transform(skeleton.binding_matrices[i]
				, ctx._world[i]
				)
				;
		}

		for (u32 i = 0; i < array::size(geometry._position_indices); ++i) {
			const u32 position_index = geometry._position_indices[i] * 3;
			const Vector3 position = {
				geometry._positions[position_index + 0],
				geometry._positions[position_index + 1],
				geometry._positions[position_index + 2]
			};
			const u32 bone_index = geometry._bone_indices[i] * Geometry::MAX_BONE_WEIGHTS;
			const u32 weight_index = geometry._weight_indices[i] * Geometry::MAX_BONE_WEIGHTS;
			ctx._points[i] = mesh_animation::skin(position
				, array::begin(ctx._palette)
				, array::begin(geometry._bones) + bone_index
				, array::begin(geometry._weights) + weight_index
				);
		}

		SampleBounds bounds;
		aabb::from_points(bounds.aabb, array::size(ctx._points), array::begin(ctx._points));
		sphere::add_points(bounds.sphere, array::size(ctx._points), array::begin(ctx._points));
		return bounds;
	}

	static s32 validate_geometry(const Geometry &geometry
		, const char *geometry_name
		, const AnimationSkeleton &skeleton
		, CompileOptions &opts
		)
	{
		const u32 num_indices = array::size(geometry._position_indices);
		RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
			, array::size(geometry._bone_indices) == num_indices
			&& array::size(geometry._weight_indices) == num_indices
			, opts
			, "Skinned geometry '%s' has mismatched vertex attribute indices"
			, geometry_name
			);

		for (u32 i = 0; i < num_indices; ++i) {
			const u32 position_index = geometry._position_indices[i] * 3;
			const u32 bone_index = geometry._bone_indices[i] * Geometry::MAX_BONE_WEIGHTS;
			const u32 weight_index = geometry._weight_indices[i] * Geometry::MAX_BONE_WEIGHTS;
			RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
				, position_index + 2 < array::size(geometry._positions)
				&& bone_index + 3 < array::size(geometry._bones)
				&& weight_index + 3 < array::size(geometry._weights)
				, opts
				, "Skinned geometry '%s' has invalid vertex attribute indices"
				, geometry_name
				);

			for (u32 w = 0; w < Geometry::MAX_BONE_WEIGHTS; ++w) {
				const f32 bone_value = geometry._bones[bone_index + w];
				RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
					, bone_value >= 0.0f && bone_value <= f32(UINT16_MAX)
					, opts
					, "Skinned geometry '%s' has invalid bone index %f"
					, geometry_name
					, bone_value
					);
				const u32 bone = (u32)bone_value;
				RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
					, f32(bone) == bone_value && bone < array::size(skeleton.local_transforms)
					, opts
					, "Skinned geometry '%s' bone index %u is outside the target skeleton"
					, geometry_name
					, bone
					);
			}
		}

		return 0;
	}

	static s32 validate_skeleton(const AnimationSkeleton &skeleton
		, const MeshAnimation &animation
		, CompileOptions &opts
		)
	{
		RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
			, array::size(skeleton.local_transforms) == array::size(skeleton.parents)
			&& array::size(skeleton.local_transforms) == array::size(skeleton.binding_matrices)
			, opts
			, "Target skeleton data is inconsistent"
			);
		for (u32 i = 0; i < array::size(skeleton.parents); ++i) {
			RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
				, skeleton.parents[i] == UINT16_MAX || skeleton.parents[i] < i
				, opts
				, "Target skeleton parent index is invalid"
				);
		}
		for (u32 i = 0; i < array::size(animation.bone_ids); ++i) {
			RETURN_IF_FALSE(MESH_ANIMATION_RESOURCE
				, animation.bone_ids[i] < array::size(skeleton.local_transforms)
				, opts
				, "Animation bone index %u is outside the target skeleton"
				, animation.bone_ids[i]
				);
		}

		return 0;
	}

	static void collect_sample_timestamps(Array<u16> &timestamps, const MeshAnimation &animation)
	{
		if (array::size(animation.keys) == 0)
			return;

		// Include every encoded pose and one uniform grid over the whole clip.
		u16 end = (u16)(animation.total_time * 1000.0f);
		array::push_back(timestamps, end);
		for (u32 i = 0; i < array::size(animation.keys); ++i) {
			const u16 ts = animation.keys[i].h.time;
			array::push_back(timestamps, ts);
			if (ts > end)
				end = ts;
		}
		for (u32 frame = 0; ; ++frame) {
			const u32 ts = (u32)((f32)frame * 1000.0f / animation.bounds_sample_rate);
			if (ts > end)
				break;
			array::push_back(timestamps, (u16)ts);
		}

		std::sort(array::begin(timestamps), array::end(timestamps));
		const u16 *timestamps_end = std::unique(array::begin(timestamps), array::end(timestamps));
		array::resize(timestamps, (u32)(timestamps_end - array::begin(timestamps)));
	}

	static s32 generate_geometry_bounds(MeshAnimationBounds &result
		, const MeshAnimation &animation
		, const AnimationSkeleton &skeleton
		, const Geometry &geometry
		, const char *geometry_name
		, const Array<u16> &timestamps
		, CompileOptions &opts
		)
	{
		s32 err = validate_geometry(geometry, geometry_name, skeleton, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);

		SampleContext ctx(default_allocator(), animation, skeleton, geometry);
		BoundsAccumulator bounds = {};
		for (u32 i = 0; i < array::size(timestamps); ++i)
			add_sample(bounds, sample_bounds(ctx, timestamps[i]));

		CE_ENSURE(bounds.valid);
		result.obb.tm = from_translation(aabb::center(bounds.aabb));
		result.obb.half_extents = (bounds.aabb.max - bounds.aabb.min) * 0.5f + splat<Vector3>(animation.bounds_epsilon);
		result.sphere = bounds.sphere;
		result.sphere.r += animation.bounds_epsilon;
		return 0;
	}

	static s32 generate_animation_bounds(MeshAnimation &animation, CompileOptions &opts)
	{
		// Bounds use the target skeleton's owning mesh, not the animation source.
		TempAllocator4096 ta;
		DynamicString skeleton_path(ta);
		skeleton_path = animation.target_skeleton;
		skeleton_path += ".mesh_skeleton";

		AnimationSkeleton skeleton(default_allocator());
		s32 err = mesh_skeleton::parse(skeleton, skeleton_path.c_str(), opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);
		err = validate_skeleton(skeleton, animation, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);

		DynamicString mesh_path(ta);
		mesh_path = skeleton.mesh_resource_name;
		mesh_path += ".mesh";
		Mesh mesh(default_allocator());
		err = mesh::parse(mesh, mesh_path.c_str(), opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);

		Array<u16> timestamps(default_allocator());
		collect_sample_timestamps(timestamps, animation);
		if (array::size(timestamps) == 0)
			return 0;

		auto geometry_cur = hash_map::begin(mesh._geometries);
		auto geometry_end = hash_map::end(mesh._geometries);
		for (; geometry_cur != geometry_end; ++geometry_cur) {
			HASH_MAP_SKIP_HOLE(mesh._geometries, geometry_cur);
			const Geometry &geometry = geometry_cur->second;
			if (!mesh::has_bones(geometry) || array::size(geometry._position_indices) == 0)
				continue;

			MeshAnimationBounds geometry_bounds = {};
			err = generate_geometry_bounds(geometry_bounds
				, animation
				, skeleton
				, geometry
				, geometry_cur->first.c_str()
				, timestamps
				, opts
				);
			ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);

			auto node_cur = hash_map::begin(mesh._nodes);
			auto node_end = hash_map::end(mesh._nodes);
			for (; node_cur != node_end; ++node_cur) {
				HASH_MAP_SKIP_HOLE(mesh._nodes, node_cur);
				if (node_cur->second._geometry != geometry_cur->first)
					continue;

				AnimationBounds node_bounds;
				node_bounds.geometry_name = node_cur->first.to_string_id();
				node_bounds.bounds = geometry_bounds;
				array::push_back(animation.bounds, node_bounds);
			}
		}

		if (array::size(animation.bounds) > 1)
			std::sort(array::begin(animation.bounds), array::end(animation.bounds));
		return 0;
	}

	static s32 write(MeshAnimation &ma, CompileOptions &opts)
	{
		MeshAnimationResource mar;
		mar.version = RESOURCE_VERSION_MESH_ANIMATION;
		mar.num_tracks = hash_map::size(ma.track_ids);
		mar.total_time = ma.total_time;
		mar.num_keys = array::size(ma.sorted_keys);
		mar.keys_offset = sizeof(mar);
		mar._pad0 = 0u;
		mar.target_skeleton = ma.target_skeleton_id;
		mar.num_bones = array::size(ma.bone_ids);
		mar.bone_ids_offset = mar.keys_offset + mar.num_keys * sizeof(AnimationKey);
		mar.num_events = array::size(ma.events);
		mar.event_times_offset = mar.bone_ids_offset + mar.num_bones * sizeof(u16);
		mar.event_names_offset = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)(mar.event_times_offset + mar.num_events * sizeof(u16)), sizeof(u32));
		mar.num_bounds = array::size(ma.bounds);
		mar.geometry_names_offset = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)(mar.event_names_offset + mar.num_events * sizeof(StringId32)), alignof(StringId32));
		mar.bounds_offset = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)(mar.geometry_names_offset + mar.num_bounds * sizeof(StringId32)), alignof(MeshAnimationBounds));

		opts.write(mar.version);
		opts.write(mar.num_tracks);
		opts.write(mar.total_time);
		opts.write(mar.num_keys);
		opts.write(mar.keys_offset);
		opts.write(mar._pad0);
		opts.write(mar.target_skeleton);
		opts.write(mar.num_bones);
		opts.write(mar.bone_ids_offset);
		opts.write(mar.num_events);
		opts.write(mar.event_times_offset);
		opts.write(mar.event_names_offset);
		opts.write(mar.num_bounds);
		opts.write(mar.geometry_names_offset);
		opts.write(mar.bounds_offset);

		for (u32 i = 0; i < array::size(ma.sorted_keys); ++i)
			opts.write(ma.sorted_keys[i]);

		for (u32 i = 0; i < array::size(ma.bone_ids); ++i)
			opts.write(ma.bone_ids[i]);

		for (u32 i = 0; i < array::size(ma.events); ++i)
			opts.write(u16(ma.events[i].time * 1000.0f));

		opts.align(sizeof(u32));
		for (u32 i = 0; i < array::size(ma.events); ++i)
			opts.write(ma.events[i].name);

		opts.align(alignof(StringId32));
		for (u32 i = 0; i < array::size(ma.bounds); ++i)
			opts.write(ma.bounds[i].geometry_name);

		opts.align(alignof(MeshAnimationBounds));
		for (u32 i = 0; i < array::size(ma.bounds); ++i)
			opts.write(ma.bounds[i].bounds);

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		MeshAnimation ma(default_allocator());

		if (opts._resource_id._id == resource_id(RESOURCE_TYPE_MESH_ANIMATION, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248))._id) {
			ma.total_time = 1.0f;
			return write(ma, opts);
		}

		s32 err = mesh_animation::parse(ma, buf, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);
		err = generate_animation_bounds(ma, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);
		return write(ma, opts);
	}

} // namespace mesh_animation_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
