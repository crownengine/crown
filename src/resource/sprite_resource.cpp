/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
#include "resource/compile_options.inl"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"

namespace crown
{
namespace sprite_resource
{
	const f32* frame_data(const SpriteResource* sr, u32 i)
	{
		CE_ENSURE(i < sr->num_frames);
		return ((f32*)&sr[1]) + 20*i;
	}

} // namespace sprite_resource

#if CROWN_CAN_COMPILE
namespace sprite_resource_internal
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region; // [x, y, w, h]
		Vector2 pivot;  // [x, y]
	};

	void parse_frame(SpriteFrame& sf, const char* json)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(obj, json);

		sf.name   = sjson::parse_string_id(obj["name"]);
		sf.region = sjson::parse_vector4(obj["region"]);
		sf.pivot  = sjson::parse_vector2(obj["pivot"]);
	}

	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, buf);

		JsonArray frames(ta);
		sjson::parse_array(frames, obj["frames"]);

		// Read width/height
		const f32 width      = sjson::parse_float(obj["width"]);
		const f32 height     = sjson::parse_float(obj["height"]);
		const u32 num_frames = array::size(frames);

		Array<f32> vertices(default_allocator());
		for (u32 i = 0; i < num_frames; ++i)
		{
			SpriteFrame sf;
			parse_frame(sf, frames[i]);

			// Generate UV coords
			const f32 u0 = (              sf.region.x) / width;
			const f32 v0 = (sf.region.w + sf.region.y) / height;
			const f32 u1 = (sf.region.z + sf.region.x) / width;
			const f32 v1 = (              sf.region.y) / height;

			// Generate positions
			f32 x0 = (              sf.region.x - sf.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y0 = (sf.region.w + sf.region.y - sf.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 x1 = (sf.region.z + sf.region.x - sf.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y1 = (              sf.region.y - sf.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;

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
			array::push_back(vertices, 0.0f); // A.y
			array::push_back(vertices, y0);   // A.z
			array::push_back(vertices, u0);   // A.u
			array::push_back(vertices, v0);   // A.v

			array::push_back(vertices, x1);   // B.x
			array::push_back(vertices, 0.0f); // B.y
			array::push_back(vertices, y0);   // B.z
			array::push_back(vertices, u1);   // B.u
			array::push_back(vertices, v0);   // B.v

			array::push_back(vertices, x1);   // C.x
			array::push_back(vertices, 0.0f); // C.y
			array::push_back(vertices, y1);   // C.z
			array::push_back(vertices, u1);   // C.u
			array::push_back(vertices, v1);   // C.v

			array::push_back(vertices, x0);   // D.x
			array::push_back(vertices, 0.0f); // D.y
			array::push_back(vertices, y1);   // D.z
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
		aabb.min.y = -0.25f;
		aabb.max.y =  0.25f;

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
#endif // CROWN_CAN_COMPILE

namespace sprite_animation_resource
{
	const u32* frames(const SpriteAnimationResource* sar)
	{
		return (u32*)&sar[1];
	}

} // namespace sprite_animation_resource

#if CROWN_CAN_COMPILE
namespace sprite_animation_resource_internal
{
	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray  object_frames(ta);

		Array<u32> frames(default_allocator());
		float total_time = 0.0f;

		sjson::parse(obj, buf);
		sjson::parse_array(object_frames, obj["frames"]);

		array::resize(frames, array::size(object_frames));
		for (u32 i = 0; i < array::size(object_frames); ++i)
			frames[i] = (u32)sjson::parse_float(object_frames[i]);

		total_time = sjson::parse_float(obj["total_time"]);

		// Write
		SpriteAnimationResource sar;
		sar.version = RESOURCE_HEADER(RESOURCE_VERSION_SPRITE_ANIMATION);
		sar.num_frames = array::size(frames);
		sar.total_time = total_time;

		opts.write(sar.version);
		opts.write(sar.num_frames);
		opts.write(sar.total_time);

		for (u32 i = 0; i < array::size(frames); i++)
			opts.write(frames[i]);

		return 0;
	}

} // namespace sprite_animation_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
