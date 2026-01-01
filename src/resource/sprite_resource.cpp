/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/vector2.inl"
#include "core/math/vector4.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include <algorithm>

LOG_SYSTEM(SPRITE_ANIMATION_RESOURCE, "sprite_animation_resource")

namespace crown
{
namespace sprite_resource
{
	const f32 *frame_data(const SpriteResource *sr, u32 i)
	{
		CE_ENSURE(i < sr->num_frames);
		return ((f32 *)&sr[1]) + 20*i;
	}

} // namespace sprite_resource

#if CROWN_CAN_COMPILE
namespace sprite_resource_internal
{
	struct SpriteFrame
	{
		u32 index;
		StringId32 name;
		Vector4 region; // [x, y, w, h]
		Vector2 pivot;  // [x, y]
	};

	s32 parse_frames(Array<SpriteFrame> &sprite_frames, const JsonArray &frames, CompileOptions &opts)
	{
		for (u32 ii = 0; ii < array::size(frames); ++ii) {
			TempAllocator512 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, frames[ii]), opts);

			SpriteFrame sf;
			sf.name   = RETURN_IF_ERROR(sjson::parse_string_id(obj["name"]), opts);
			sf.region = RETURN_IF_ERROR(sjson::parse_vector4(obj["region"]), opts);
			sf.pivot  = RETURN_IF_ERROR(sjson::parse_vector2(obj["pivot"]), opts);

			if (json_object::has(obj, "index")) {
				sf.index = RETURN_IF_ERROR(sjson::parse_int(obj["index"]), opts);
			} else {
				sf.index = ii;
			}

			array::push_back(sprite_frames, sf);
		}

		std::sort(array::begin(sprite_frames)
			, array::end(sprite_frames)
			, [](const SpriteFrame &frame_a, const SpriteFrame &frame_b) {
				return frame_a.index < frame_b.index;
			});

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		JsonArray frames(ta);
		RETURN_IF_ERROR(sjson::parse_array(frames, obj["frames"]), opts);

		// Read width/height
		const f32 width      = RETURN_IF_ERROR(sjson::parse_float(obj["width"]), opts);
		const f32 height     = RETURN_IF_ERROR(sjson::parse_float(obj["height"]), opts);
		const u32 num_frames = array::size(frames);

		// Parse frames.
		Array<SpriteFrame> sprite_frames(default_allocator());
		s32 err = parse_frames(sprite_frames, frames, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		// Fill verices.
		Array<f32> vertices(default_allocator());
		for (u32 ii = 0; ii < num_frames; ++ii) {
			const SpriteFrame &sf = sprite_frames[ii];

			// Generate UV coords
			const f32 u0 = (sf.region.x) / width;
			const f32 v0 = (sf.region.w + sf.region.y) / height;
			const f32 u1 = (sf.region.z + sf.region.x) / width;
			const f32 v1 = (sf.region.y) / height;

			// Generate positions
			f32 x0 = (sf.region.x - sf.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y0 = (sf.region.w + sf.region.y - sf.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 x1 = (sf.region.z + sf.region.x - sf.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y1 = (sf.region.y - sf.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;

			// Invert Y axis
			y0 = y0 == 0.0f ? y0 : -y0;
			y1 = y1 == 0.0f ? y1 : -y1;

			// Output vertex data
			//
			// D -- C
			// |    |
			// A -- B
			//
			array::push_back(vertices, x0);   // A.x
			array::push_back(vertices, y0);   // A.y
			array::push_back(vertices, 0.0f); // A.z
			array::push_back(vertices, u0);   // A.u
			array::push_back(vertices, v0);   // A.v

			array::push_back(vertices, x1);   // B.x
			array::push_back(vertices, y0);   // B.y
			array::push_back(vertices, 0.0f); // B.z
			array::push_back(vertices, u1);   // B.u
			array::push_back(vertices, v0);   // B.v

			array::push_back(vertices, x1);   // C.x
			array::push_back(vertices, y1);   // C.y
			array::push_back(vertices, 0.0f); // C.z
			array::push_back(vertices, u1);   // C.u
			array::push_back(vertices, v1);   // C.v

			array::push_back(vertices, x0);   // D.x
			array::push_back(vertices, y1);   // D.y
			array::push_back(vertices, 0.0f); // D.z
			array::push_back(vertices, u0);   // D.u
			array::push_back(vertices, v1);   // D.v
		}

		const u32 num_vertices = array::size(vertices) / 5; // 5 components per vertex

		AABB aabb;
		aabb::from_points(aabb
			, num_vertices
			, sizeof(vertices[0])*5
			, array::begin(vertices)
			);
		// Enforce some thickness
		aabb.min.z = -0.25f;
		aabb.max.z =  0.25f;

		OBB obb;
		obb.tm = from_quaternion_translation(QUATERNION_IDENTITY, aabb::center(aabb));
		obb.half_extents = (aabb.max - aabb.min) * 0.5f;

		// Write
		SpriteResource sr;
		sr.version = RESOURCE_HEADER(RESOURCE_VERSION_SPRITE);
		sr.obb = obb;
		sr.num_frames = num_frames;
		sr.num_verts = num_vertices;

		opts.write(sr.version);
		opts.write(sr.obb);
		opts.write(sr.num_frames);

		opts.write(sr.num_verts);
		for (u32 i = 0; i < array::size(vertices); i++)
			opts.write(vertices[i]);

		return 0;
	}

} // namespace sprite_resource_internal
#endif // if CROWN_CAN_COMPILE

namespace sprite_animation_resource
{
	const u32 *frames(const SpriteAnimationResource *sar)
	{
		return (u32 *)&sar[1];
	}

} // namespace sprite_animation_resource

#if CROWN_CAN_COMPILE
struct SpriteAnimationFrame
{
	u32 frame; // Sprite frame.
	u32 index; // Sorting index.
};

namespace sprite_animation_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray object_frames(ta);

		Array<SpriteAnimationFrame> frames(default_allocator());
		float total_time = 0.0f;

		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);
		RETURN_IF_ERROR(sjson::parse_array(object_frames, obj["frames"]), opts);

		array::resize(frames, array::size(object_frames));
		for (u32 i = 0; i < array::size(object_frames); ++i) {
			if (sjson::type(object_frames[i]) == JsonValueType::NUMBER) {
				frames[i].frame = (u32)RETURN_IF_ERROR(sjson::parse_float(object_frames[i]), opts);
				frames[i].index = i;
			} else {
				JsonObject animation_frame_obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(animation_frame_obj, object_frames[i]), opts);

				if (json_object::has(animation_frame_obj, "frame")) {
					frames[i].frame = (u32)RETURN_IF_ERROR(sjson::parse_float(animation_frame_obj["frame"]), opts);
				} else {
					frames[i].frame = 0; // Crown 0.48 shipped with malformed .sprite_animtion files.
				}

				frames[i].index = (u32)RETURN_IF_ERROR(sjson::parse_float(animation_frame_obj["index"]), opts);
			}
		}

		std::sort(array::begin(frames)
			, array::end(frames)
			, [](const SpriteAnimationFrame &frame_a, const SpriteAnimationFrame &frame_b) {
				return frame_a.index < frame_b.index;
			});

		if (json_object::has(obj, "total_time"))
			logw(SPRITE_ANIMATION_RESOURCE, "'total_time' property is deprecated, use 'frames_per_second' instead");

		if (json_object::has(obj, "frames_per_second")) {
			f32 fps = RETURN_IF_ERROR(sjson::parse_float(obj["frames_per_second"]), opts);
			total_time = array::size(frames) / fps;
		} else {
			total_time = RETURN_IF_ERROR(sjson::parse_float(obj["total_time"]), opts);
		}

		// Write
		SpriteAnimationResource sar;
		sar.version = RESOURCE_HEADER(RESOURCE_VERSION_SPRITE_ANIMATION);
		sar.num_frames = array::size(frames);
		sar.total_time = total_time;

		opts.write(sar.version);
		opts.write(sar.num_frames);
		opts.write(sar.total_time);

		for (u32 i = 0; i < array::size(frames); i++)
			opts.write(frames[i].frame);

		return 0;
	}

} // namespace sprite_animation_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
