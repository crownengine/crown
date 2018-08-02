/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/math/aabb.h"
#include "core/math/vector2.h"
#include "core/math/vector4.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/string.h"
#include "resource/compile_options.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"

namespace crown
{
namespace sprite_resource_internal
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region; // [x, y, w, h]
		Vector2 pivot;  // [x, y]
	};

	void parse_frame(const char* json, SpriteFrame& frame)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		frame.name   = sjson::parse_string_id(obj["name"]);
		frame.region = sjson::parse_vector4(obj["region"]);
		frame.pivot  = sjson::parse_vector2(obj["pivot"]);
	}

	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		JsonArray frames(ta);
		sjson::parse_array(object["frames"], frames);

		// Read width/height
		const f32 width      = sjson::parse_float(object["width" ]);
		const f32 height     = sjson::parse_float(object["height"]);
		const u32 num_frames = array::size(frames);

		Array<f32> vertices(default_allocator());
		for (u32 i = 0; i < num_frames; ++i)
		{
			SpriteFrame frame;
			parse_frame(frames[i], frame);

			const SpriteFrame& fd = frame;

			// Compute uv coords
			const f32 u0 = (              fd.region.x) / width;
			const f32 v0 = (fd.region.w + fd.region.y) / height;
			const f32 u1 = (fd.region.z + fd.region.x) / width;
			const f32 v1 = (              fd.region.y) / height;

			// Compute positions
			f32 x0 = (              fd.region.x - fd.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y0 = (fd.region.w + fd.region.y - fd.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 x1 = (fd.region.z + fd.region.x - fd.pivot.x) / CROWN_DEFAULT_PIXELS_PER_METER;
			f32 y1 = (              fd.region.y - fd.pivot.y) / CROWN_DEFAULT_PIXELS_PER_METER;

			// Invert Y axis
			y0 = y0 == 0.0f ? y0 : -y0;
			y1 = y1 == 0.0f ? y1 : -y1;

			array::push_back(vertices, x0);
			array::push_back(vertices, 0.0f);
			array::push_back(vertices, y0);
			array::push_back(vertices, u0);
			array::push_back(vertices, v0);

			array::push_back(vertices, x1);
			array::push_back(vertices, 0.0f);
			array::push_back(vertices, y0);
			array::push_back(vertices, u1);
			array::push_back(vertices, v0);

			array::push_back(vertices, x1);
			array::push_back(vertices, 0.0f);
			array::push_back(vertices, y1);
			array::push_back(vertices, u1);
			array::push_back(vertices, v1);

			array::push_back(vertices, x0);
			array::push_back(vertices, 0.0f);
			array::push_back(vertices, y1);
			array::push_back(vertices, u0);
			array::push_back(vertices, v1);
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
		obb.tm = matrix4x4(QUATERNION_IDENTITY, aabb::center(aabb));
		obb.half_extents = (aabb.max - aabb.min) * 0.5f;

		// Write
		SpriteResource sr;
		sr.version = RESOURCE_VERSION_SPRITE;
		sr.obb = obb;
		sr.num_verts = num_vertices;

		opts.write(sr.version);
		opts.write(sr.obb);

		opts.write(sr.num_verts);
		for (u32 i = 0; i < array::size(vertices); i++)
			opts.write(vertices[i]);
	}

} // namespace sprite_resource_internal

namespace sprite_resource
{
	const f32* frame_data(const SpriteResource* sr, u32 i)
	{
		return ((f32*)&sr[1]) + 20*i;
	}

} // namespace sprite_resource

namespace sprite_animation_resource_internal
{
	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonArray  object_frames(ta);

		Array<u32> frames(default_allocator());
		float total_time = 0.0f;

		sjson::parse(buf, object);
		sjson::parse_array(object["frames"], object_frames);

		array::resize(frames, array::size(object_frames));
		for (u32 i = 0; i < array::size(object_frames); ++i)
			frames[i] = (u32)sjson::parse_float(object_frames[i]);

		total_time = sjson::parse_float(object["total_time"]);

		// Write
		SpriteAnimationResource sar;
		sar.version = RESOURCE_VERSION_SPRITE_ANIMATION;
		sar.num_frames = array::size(frames);
		sar.total_time = total_time;

		opts.write(sar.version);
		opts.write(sar.num_frames);
		opts.write(sar.total_time);

		for (u32 i = 0; i < array::size(frames); i++)
			opts.write(frames[i]);
	}

} // namespace sprite_animation_resource_internal

namespace sprite_animation_resource
{
	const u32* frames(const SpriteAnimationResource* sar)
	{
		return (u32*)&sar[1];
	}

} // namespace sprite_animation_resource

} // namespace crown
