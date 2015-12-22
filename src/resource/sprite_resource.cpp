/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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
#include "njson.h"
#include "map.h"

namespace crown
{
namespace sprite_resource
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region;		// [x0, y0, x1, y1]
		Vector2 scale;		// [Sx, Sy]
		Vector2 offset;		// [Ox, Oy]
	};

	void parse_frame(const char* json, SpriteFrame& frame)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		njson::parse(json, obj);

		frame.name   = njson::parse_string_id(obj["name"]);
		frame.region = njson::parse_vector4(obj["region"]);
		frame.offset = njson::parse_vector2(obj["offset"]);
		frame.scale  = njson::parse_vector2(obj["scale"]);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		njson::parse(buf, object);

		JsonArray frames(ta);
		njson::parse_array(object["frames"], frames);

		// Read width/height
		const float width         = parse_float(object["width" ]);
		const float height        = parse_float(object["height"]);
		const uint32_t num_frames = array::size(frames);

		Array<float> vertices(default_allocator());
		Array<uint16_t> indices(default_allocator());
		uint32_t num_idx = 0;
		for (uint32_t i = 0; i < num_frames; ++i)
		{
			SpriteFrame frame;
			parse_frame(frames[i], frame);

			const SpriteFrame& fd = frame;

			// Compute uv coords
			const float u0 = fd.region.x / width;
			const float v0 = fd.region.y / height;
			const float u1 = (fd.region.x + fd.region.z) / width;
			const float v1 = (fd.region.y + fd.region.w) / height;

			// Compute positions
			const float w = fd.region.z / CROWN_DEFAULT_PIXELS_PER_METER;
			const float h = fd.region.w / CROWN_DEFAULT_PIXELS_PER_METER;

			const float x0 = fd.scale.x * (-w * 0.5f) + fd.offset.x;
			const float y0 = fd.scale.y * (-h * 0.5f) + fd.offset.y;
			const float x1 = fd.scale.x * ( w * 0.5f) + fd.offset.x;
			const float y1 = fd.scale.y * ( h * 0.5f) + fd.offset.y;

			array::push_back(vertices, x0); array::push_back(vertices, y0); // position
			array::push_back(vertices, u0); array::push_back(vertices, v0); // uv

			array::push_back(vertices, x1); array::push_back(vertices, y0); // position
			array::push_back(vertices, u1); array::push_back(vertices, v0); // uv

			array::push_back(vertices, x1); array::push_back(vertices, y1); // position
			array::push_back(vertices, u1); array::push_back(vertices, v1); // uv

			array::push_back(vertices, x0); array::push_back(vertices, y1); // position
			array::push_back(vertices, u0); array::push_back(vertices, v1); // uv

			array::push_back(indices, uint16_t(num_idx)); array::push_back(indices, uint16_t(num_idx + 1)); array::push_back(indices, uint16_t(num_idx + 2));
			array::push_back(indices, uint16_t(num_idx)); array::push_back(indices, uint16_t(num_idx + 2)); array::push_back(indices, uint16_t(num_idx + 3));
			num_idx += 4;
		}

		const uint32_t num_vertices = array::size(vertices) / 4; // 4 components per vertex
		const uint32_t num_indices = array::size(indices);

		// Write
		opts.write(SPRITE_VERSION);

		opts.write(num_vertices);
		for (uint32_t i = 0; i < array::size(vertices); i++)
		{
			opts.write(vertices[i]);
		}

		opts.write(num_indices);
		for (uint32_t i = 0; i < array::size(indices); i++)
		{
			opts.write(indices[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		uint32_t version;
		br.read(version);

		uint32_t num_verts;
		br.read(num_verts);
		const bgfx::Memory* vbmem = bgfx::alloc(num_verts * sizeof(float) * 4);
		br.read(vbmem->data, num_verts * sizeof(float) * 4);

		uint32_t num_inds;
		br.read(num_inds);
		const bgfx::Memory* ibmem = bgfx::alloc(num_inds * sizeof(uint16_t));
		br.read(ibmem->data, num_inds * sizeof(uint16_t));

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
	void parse_animation(const char* json, Array<SpriteAnimationName>& names, Array<SpriteAnimationData>& anim_data, Array<uint32_t>& frames)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		njson::parse(json, obj);

		SpriteAnimationName san;
		san.id = njson::parse_string_id(obj["name"]);

		JsonArray obj_frames(ta);
		njson::parse_array(obj["frames"], obj_frames);

		const uint32_t num_frames = array::size(obj_frames);

		SpriteAnimationData sad;
		sad.num_frames  = num_frames;
		sad.first_frame = array::size(frames);
		sad.time        = njson::parse_float(obj["time"]);

		// Read frames
		for (uint32_t ff = 0; ff < num_frames; ++ff)
			array::push_back(frames, (uint32_t)njson::parse_int(obj_frames[ff]));

		array::push_back(names, san);
		array::push_back(anim_data, sad);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		njson::parse(buf, object);

		JsonArray animations(ta);
		njson::parse_array(object["animations"], animations);

		Array<SpriteAnimationName> anim_names(default_allocator());
		Array<SpriteAnimationData> anim_data(default_allocator());
		Array<uint32_t> anim_frames(default_allocator());

		const uint32_t num_animations = array::size(animations);
		for (uint32_t i = 0; i < num_animations; ++i)
		{
			parse_animation(animations[i], anim_names, anim_data, anim_frames);
		}

		SpriteAnimationResource sar;
		sar.version = SPRITE_ANIMATION_VERSION;
		sar.num_animations = array::size(anim_names);
		sar.num_frames = array::size(anim_frames);
		sar.frames_offset = uint32_t(sizeof(SpriteAnimationResource) +
					sizeof(SpriteAnimationName) * array::size(anim_names) +
					sizeof(SpriteAnimationData) * array::size(anim_data));

		opts.write(sar.version);
		opts.write(sar.num_animations);
		opts.write(sar.num_frames);
		opts.write(sar.frames_offset);

		for (uint32_t i = 0; i < array::size(anim_names); i++)
		{
			opts.write(anim_names[i].id);
		}

		for (uint32_t i = 0; i < array::size(anim_data); i++)
		{
			opts.write(anim_data[i].num_frames);
			opts.write(anim_data[i].first_frame);
			opts.write(anim_data[i].time);
		}

		for (uint32_t i = 0; i < array::size(anim_frames); i++)
		{
			opts.write(anim_frames[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == SPRITE_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

	const SpriteAnimationData* get_animation(const SpriteAnimationResource* sar, StringId32 name)
	{
		const uint32_t num = sar->num_animations;
		const SpriteAnimationName* begin = (SpriteAnimationName*) ((char*) sar + sizeof(*sar));
		const SpriteAnimationData* data = (SpriteAnimationData*) ((char*) sar + sizeof(*sar) + sizeof(SpriteAnimationName) * num);

		for (uint32_t i = 0; i < num; i++)
		{
			if (begin[i].id == name)
				return &data[i];
		}

		return NULL;
	}

	const uint32_t* get_animation_frames(const SpriteAnimationResource* sar)
	{
		return (uint32_t*) ((char*) sar + sar->frames_offset);
	}
} // namespace sprite_animation_resource
} // namespace crown
