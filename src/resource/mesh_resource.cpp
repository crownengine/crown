/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "dynamic_string.h"
#include "filesystem.h"
#include "json_parser.h"
#include "log.h"
#include "mesh_resource.h"
#include "temp_allocator.h"
#include "vector3.h"
#include "resource_manager.h"
#include "compile_options.h"
#include "vector2.h"
#include "reader_writer.h"

namespace crown
{
namespace mesh_resource
{
	struct MeshVertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;

		bool operator==(const MeshVertex& other)
		{
			return position == other.position
				&& normal == other.normal
				&& texcoord == other.texcoord;
		}
	};

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		// Read data arrays
		JSONElement position = root.key_or_nil("position");
		CE_ASSERT(!position.is_nil(), "Bad mesh: array 'position' not found.");
		JSONElement normal = root.key_or_nil("normal");
		JSONElement texcoord = root.key_or_nil("texcoord");

		Array<float> positions(default_allocator());
		Array<float> normals(default_allocator());
		Array<float> texcoords(default_allocator());

		position.to_array(positions);

		bool has_normal = false;
		bool has_texcoord = false;

		if (!normal.is_nil())
		{
			has_normal = true;
			normal.to_array(normals);
		}

		if (!texcoord.is_nil())
		{
			has_texcoord = true;
			texcoord.to_array(texcoords);
		}

		// Read index arrays
		JSONElement index = root.key("index");

		Array<uint16_t> position_index(default_allocator());
		Array<uint16_t> normal_index(default_allocator());
		Array<uint16_t> texcoord_index(default_allocator());

		int ii = 0;
		index[ii].to_array(position_index);
		++ii;

		if (has_normal)
		{
			index[ii].to_array(normal_index);
			++ii;
		}

		if (has_texcoord)
		{
			index[ii].to_array(texcoord_index);
			++ii;
		}

		Array<MeshVertex> vertices(default_allocator());
		Array<uint16_t> indices(default_allocator());

		// Generate vb/ib
		uint32_t idx = 0;
		for (uint32_t i = 0; i < array::size(position_index); i++)
		{
			MeshVertex v;

			uint16_t p_idx = position_index[i] * 3;
			v.position = vector3(positions[p_idx], positions[p_idx + 1], positions[p_idx + 2]);

			if (has_normal)
			{
				uint16_t n_idx = normal_index[i] * 3;
				v.normal = vector3(normals[n_idx], normals[n_idx + 1], normals[n_idx + 2]);
			}
			if (has_texcoord)
			{
				uint16_t t_idx = texcoord_index[i] * 2;
				v.texcoord = vector2(texcoords[t_idx], texcoords[t_idx + 1]);
			}

			uint32_t f_idx = 0;
			bool found = false;
			for (; f_idx < array::size(vertices); f_idx++)
			{
				if (vertices[f_idx] == v)
				{
					found = true;
					break;
				}
			}

			if (found)
			{
				array::push_back(indices, (uint16_t) f_idx);
			}
			else
			{
				array::push_back(vertices, v);
				array::push_back(indices, (uint16_t) idx);
				idx++;
			}
		}

		bgfx::VertexDecl decl;
		decl.begin();
		decl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

		if (has_normal)
			decl.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, false);

		if (has_texcoord)
			decl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);

		decl.end();

		// Write
		opts.write(MESH_VERSION);
		opts.write(decl);
		opts.write(array::size(vertices));
		for (uint32_t i = 0; i < array::size(vertices); ++i)
		{
			opts.write(vertices[i].position);

			if (has_normal)
				opts.write(vertices[i].normal);

			if (has_texcoord)
				opts.write(vertices[i].texcoord);
		}

		opts.write(array::size(indices));
		for (uint32_t i = 0; i < array::size(indices); ++i)
		{
			opts.write(indices[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		uint32_t version;
		br.read(version);
		CE_ASSERT(version == MESH_VERSION, "Wrong version");

		bgfx::VertexDecl decl;
		br.read(decl);

		uint32_t num_verts;
		br.read(num_verts);
		const bgfx::Memory* vbmem = bgfx::alloc(num_verts * decl.getStride());
		br.read(vbmem->data, num_verts * decl.getStride());

		uint32_t num_inds;
		br.read(num_inds);
		const bgfx::Memory* ibmem = bgfx::alloc(num_inds * sizeof(uint16_t));
		br.read(ibmem->data, num_inds * sizeof(uint16_t));

		MeshResource* mr = (MeshResource*)a.allocate(sizeof(MeshResource));
		mr->decl = decl;
		mr->vbmem = vbmem;
		mr->ibmem = ibmem;

		return mr;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		MeshResource* mr = (MeshResource*)rm.get(MESH_TYPE, id);

		mr->vb = bgfx::createVertexBuffer(mr->vbmem, mr->decl);
		mr->ib = bgfx::createIndexBuffer(mr->ibmem);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		MeshResource* mr = (MeshResource*)rm.get(MESH_TYPE, id);

		bgfx::destroyVertexBuffer(mr->vb);
		bgfx::destroyIndexBuffer(mr->ib);
	}

	void unload(Allocator& a, void* res)
	{
		a.deallocate(res);
	}
} // namespace mesh_resource
} // namespace crown
