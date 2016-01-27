/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "sprite_resource.h"
#include "string_utils.h"
#include "array.h"
#include "config.h"
#include "reader_writer.h"
#include "vector2.h"
#include "vector4.h"
#include "resource_manager.h"
#include "compile_options.h"
#include "sjson.h"
#include "map.h"

namespace crown
{
namespace sprite_resource
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region;  // [x0, y0, x1, y1]
		Vector2 scale;   // [Sx, Sy]
		Vector2 offset;  // [Ox, Oy]
	};

	void parse_frame(const char* json, SpriteFrame& frame)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		frame.name   = sjson::parse_string_id(obj["name"]);
		frame.region = sjson::parse_vector4(obj["region"]);
		frame.offset = sjson::parse_vector2(obj["offset"]);
		frame.scale  = sjson::parse_vector2(obj["scale"]);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

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
		Array<u16> indices(default_allocator());
		u32 num_idx = 0;
		for (u32 i = 0; i < num_frames; ++i)
		{
			SpriteFrame frame;
			parse_frame(frames[i], frame);

			const SpriteFrame& fd = frame;

			// Compute uv coords
			const f32 u0 = fd.region.x / width;
			const f32 v0 = fd.region.y / height;
			const f32 u1 = (fd.region.x + fd.region.z) / width;
			const f32 v1 = (fd.region.y + fd.region.w) / height;

			// Compute positions
			const f32 w = fd.region.z / CROWN_DEFAULT_PIXELS_PER_METER;
			const f32 h = fd.region.w / CROWN_DEFAULT_PIXELS_PER_METER;

			const f32 x0 = fd.scale.x * (-w * 0.5f) + fd.offset.x;
			const f32 y0 = fd.scale.y * (-h * 0.5f) + fd.offset.y;
			const f32 x1 = fd.scale.x * ( w * 0.5f) + fd.offset.x;
			const f32 y1 = fd.scale.y * ( h * 0.5f) + fd.offset.y;

			array::push_back(vertices, x0); array::push_back(vertices, y0); // position
			array::push_back(vertices, u0); array::push_back(vertices, v0); // uv

			array::push_back(vertices, x1); array::push_back(vertices, y0); // position
			array::push_back(vertices, u1); array::push_back(vertices, v0); // uv

			array::push_back(vertices, x1); array::push_back(vertices, y1); // position
			array::push_back(vertices, u1); array::push_back(vertices, v1); // uv

			array::push_back(vertices, x0); array::push_back(vertices, y1); // position
			array::push_back(vertices, u0); array::push_back(vertices, v1); // uv

			array::push_back(indices, u16(num_idx)); array::push_back(indices, u16(num_idx + 1)); array::push_back(indices, u16(num_idx + 2));
			array::push_back(indices, u16(num_idx)); array::push_back(indices, u16(num_idx + 2)); array::push_back(indices, u16(num_idx + 3));
			num_idx += 4;
		}

		const u32 num_vertices = array::size(vertices) / 4; // 4 components per vertex
		const u32 num_indices = array::size(indices);

		// Write
		opts.write(SPRITE_VERSION);

		opts.write(num_vertices);
		for (u32 i = 0; i < array::size(vertices); i++)
		{
			opts.write(vertices[i]);
		}

		opts.write(num_indices);
		for (u32 i = 0; i < array::size(indices); i++)
		{
			opts.write(indices[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);

		u32 num_verts;
		br.read(num_verts);
		const bgfx::Memory* vbmem = bgfx::alloc(num_verts * sizeof(f32) * 4);
		br.read(vbmem->data, num_verts * sizeof(f32) * 4);

		u32 num_inds;
		br.read(num_inds);
		const bgfx::Memory* ibmem = bgfx::alloc(num_inds * sizeof(u16));
		br.read(ibmem->data, num_inds * sizeof(u16));

		SpriteResource* so = (SpriteResource*) a.allocate(sizeof(SpriteResource));
		so->vbmem = vbmem;
		so->ibmem = ibmem;

		return so;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		SpriteResource* so = (SpriteResource*) rm.get(SPRITE_TYPE, id);

		bgfx::VertexDecl decl;
		decl.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false)
			.end();

		so->vb = bgfx::createVertexBuffer(so->vbmem, decl);
		so->ib = bgfx::createIndexBuffer(so->ibmem);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		SpriteResource* so = (SpriteResource*) rm.get(SPRITE_TYPE, id);

		bgfx::destroyVertexBuffer(so->vb);
		bgfx::destroyIndexBuffer(so->ib);
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}
} // namespace sprite_resource

namespace sprite_animation_resource
{
	void parse_animation(const char* json, Array<SpriteAnimationName>& names, Array<SpriteAnimationData>& anim_data, Array<u32>& frames)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		SpriteAnimationName san;
		san.id = sjson::parse_string_id(obj["name"]);

		JsonArray obj_frames(ta);
		sjson::parse_array(obj["frames"], obj_frames);

		const u32 num_frames = array::size(obj_frames);

		SpriteAnimationData sad;
		sad.num_frames  = num_frames;
		sad.first_frame = array::size(frames);
		sad.time        = sjson::parse_float(obj["time"]);

		// Read frames
		for (u32 ff = 0; ff < num_frames; ++ff)
			array::push_back(frames, (u32)sjson::parse_int(obj_frames[ff]));

		array::push_back(names, san);
		array::push_back(anim_data, sad);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		JsonArray animations(ta);
		sjson::parse_array(object["animations"], animations);

		Array<SpriteAnimationName> anim_names(default_allocator());
		Array<SpriteAnimationData> anim_data(default_allocator());
		Array<u32> anim_frames(default_allocator());

		const u32 num_animations = array::size(animations);
		for (u32 i = 0; i < num_animations; ++i)
		{
			parse_animation(animations[i], anim_names, anim_data, anim_frames);
		}

		SpriteAnimationResource sar;
		sar.version = SPRITE_ANIMATION_VERSION;
		sar.num_animations = array::size(anim_names);
		sar.num_frames = array::size(anim_frames);
		sar.frames_offset = u32(sizeof(SpriteAnimationResource) +
					sizeof(SpriteAnimationName) * array::size(anim_names) +
					sizeof(SpriteAnimationData) * array::size(anim_data));

		opts.write(sar.version);
		opts.write(sar.num_animations);
		opts.write(sar.num_frames);
		opts.write(sar.frames_offset);

		for (u32 i = 0; i < array::size(anim_names); i++)
		{
			opts.write(anim_names[i].id);
		}

		for (u32 i = 0; i < array::size(anim_data); i++)
		{
			opts.write(anim_data[i].num_frames);
			opts.write(anim_data[i].first_frame);
			opts.write(anim_data[i].time);
		}

		for (u32 i = 0; i < array::size(anim_frames); i++)
		{
			opts.write(anim_frames[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const u32 file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(u32*)res == SPRITE_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

	const SpriteAnimationData* get_animation(const SpriteAnimationResource* sar, StringId32 name)
	{
		const u32 num = sar->num_animations;
		const SpriteAnimationName* begin = (SpriteAnimationName*) ((char*) sar + sizeof(*sar));
		const SpriteAnimationData* data = (SpriteAnimationData*) ((char*) sar + sizeof(*sar) + sizeof(SpriteAnimationName) * num);

		for (u32 i = 0; i < num; i++)
		{
			if (begin[i].id == name)
				return &data[i];
		}

		return NULL;
	}

	const u32* get_animation_frames(const SpriteAnimationResource* sar)
	{
		return (u32*) ((char*) sar + sar->frames_offset);
	}
} // namespace sprite_animation_resource
} // namespace crown
