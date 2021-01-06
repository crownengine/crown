/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh_resource.h"
#include "resource/resource_manager.h"
#include <bx/readerwriter.h>
#include <bx/error.h>
#include <vertexlayout.h> // bgfx::write, bgfx::read

namespace crown
{
struct BgfxReader : public bx::ReaderI
{
	BinaryReader* _br;

	///
	BgfxReader(BinaryReader& br)
		: _br(&br)
	{
	}

	///
	virtual ~BgfxReader()
	{
	}

	///
	virtual int32_t read(void* _data, int32_t _size, bx::Error* _err)
	{
		CE_UNUSED(_err);
		_br->read(_data, _size);
		return _size; // FIXME: return the actual number of bytes read
	}
};

/// Writer interface.
struct BgfxWriter : public bx::WriterI
{
	BinaryWriter* _bw;

	///
	BgfxWriter(BinaryWriter& bw)
		: _bw(&bw)
	{
	}

	///
	virtual ~BgfxWriter()
	{
	}

	///
	virtual int32_t write(const void* _data, int32_t _size, bx::Error* _err)
	{
		CE_UNUSED(_err);
		_bw->write(_data, _size);
		return _size; // FIXME: return the actual number of bytes written
	}
};

MeshResource::MeshResource(Allocator& a)
	: geometry_names(a)
	, geometries(a)
{
}

const MeshGeometry* MeshResource::geometry(StringId32 name) const
{
	for (u32 i = 0; i < array::size(geometry_names); ++i)
	{
		if (geometry_names[i] == name)
			return geometries[i];
	}

	CE_FATAL("Mesh name not found");
	return NULL;
}

namespace mesh_resource_internal
{
	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);
		CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_MESH), "Wrong version");

		u32 num_geoms;
		br.read(num_geoms);

		MeshResource* mr = CE_NEW(a, MeshResource)(a);
		array::resize(mr->geometry_names, num_geoms);
		array::resize(mr->geometries, num_geoms);

		for (u32 i = 0; i < num_geoms; ++i)
		{
			StringId32 name;
			br.read(name);

			bgfx::VertexLayout layout;
			BgfxReader reader(br);
			bgfx::read(&reader, layout);

			OBB obb;
			br.read(obb);

			u32 num_verts;
			br.read(num_verts);

			u32 stride;
			br.read(stride);

			u32 num_inds;
			br.read(num_inds);

			const u32 vsize = num_verts*stride;
			const u32 isize = num_inds*sizeof(u16);

			const u32 size = sizeof(MeshGeometry) + vsize + isize;

			MeshGeometry* mg = (MeshGeometry*)a.allocate(size);
			mg->obb             = obb;
			mg->layout          = layout;
			mg->vertex_buffer   = BGFX_INVALID_HANDLE;
			mg->index_buffer    = BGFX_INVALID_HANDLE;
			mg->vertices.num    = num_verts;
			mg->vertices.stride = stride;
			mg->vertices.data   = (char*)&mg[1];
			mg->indices.num     = num_inds;
			mg->indices.data    = mg->vertices.data + vsize;

			br.read(mg->vertices.data, vsize);
			br.read(mg->indices.data, isize);

			mr->geometry_names[i] = name;
			mr->geometries[i] = mg;
		}

		return mr;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		MeshResource* mr = (MeshResource*)rm.get(RESOURCE_TYPE_MESH, id);

		for (u32 i = 0; i < array::size(mr->geometries); ++i)
		{
			MeshGeometry& mg = *mr->geometries[i];

			const u32 vsize = mg.vertices.num * mg.vertices.stride;
			const u32 isize = mg.indices.num * sizeof(u16);

			const bgfx::Memory* vmem = bgfx::makeRef(mg.vertices.data, vsize);
			const bgfx::Memory* imem = bgfx::makeRef(mg.indices.data, isize);

			bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(vmem, mg.layout);
			bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(imem);
			CE_ASSERT(bgfx::isValid(vbh), "Invalid vertex buffer");
			CE_ASSERT(bgfx::isValid(ibh), "Invalid index buffer");

			mg.vertex_buffer = vbh;
			mg.index_buffer  = ibh;
		}
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		MeshResource* mr = (MeshResource*)rm.get(RESOURCE_TYPE_MESH, id);

		for (u32 i = 0; i < array::size(mr->geometries); ++i)
		{
			MeshGeometry& mg = *mr->geometries[i];
			bgfx::destroy(mg.vertex_buffer);
			bgfx::destroy(mg.index_buffer);
		}
	}

	void unload(Allocator& a, void* res)
	{
		MeshResource* mr = (MeshResource*)res;

		for (u32 i = 0; i < array::size(mr->geometries); ++i)
		{
			a.deallocate(mr->geometries[i]);
		}
		CE_DELETE(a, (MeshResource*)res);
	}

} // namespace mesh_resource_internal

#if CROWN_CAN_COMPILE
namespace mesh_resource_internal
{
	static void parse_float_array(Array<f32>& output, const char* json)
	{
		TempAllocator4096 ta;
		JsonArray floats(ta);
		sjson::parse_array(floats, json);

		array::resize(output, array::size(floats));
		for (u32 i = 0; i < array::size(floats); ++i)
			output[i] = sjson::parse_float(floats[i]);
	}

	static void parse_index_array(Array<u16>& output, const char* json)
	{
		TempAllocator4096 ta;
		JsonArray indices(ta);
		sjson::parse_array(indices, json);

		array::resize(output, array::size(indices));
		for (u32 i = 0; i < array::size(indices); ++i)
			output[i] = (u16)sjson::parse_int(indices[i]);
	}

	struct MeshCompiler
	{
		CompileOptions& _opts;

		Array<f32> _positions;
		Array<f32> _normals;
		Array<f32> _uvs;
		Array<f32> _tangents;
		Array<f32> _binormals;

		Array<u16> _position_indices;
		Array<u16> _normal_indices;
		Array<u16> _uv_indices;
		Array<u16> _tangent_indices;
		Array<u16> _binormal_indices;

		u32 _vertex_stride;
		Array<char> _vertex_buffer;
		Array<u16> _index_buffer;

		AABB _aabb;
		OBB _obb;

		bgfx::VertexLayout _layout;

		bool _has_normal;
		bool _has_uv;

		explicit MeshCompiler(CompileOptions& opts)
			: _opts(opts)
			, _positions(default_allocator())
			, _normals(default_allocator())
			, _uvs(default_allocator())
			, _tangents(default_allocator())
			, _binormals(default_allocator())
			, _position_indices(default_allocator())
			, _normal_indices(default_allocator())
			, _uv_indices(default_allocator())
			, _tangent_indices(default_allocator())
			, _binormal_indices(default_allocator())
			, _vertex_stride(0)
			, _vertex_buffer(default_allocator())
			, _index_buffer(default_allocator())
			, _has_normal(false)
			, _has_uv(false)
		{
		}

		void reset()
		{
			array::clear(_positions);
			array::clear(_normals);
			array::clear(_uvs);
			array::clear(_tangents);
			array::clear(_binormals);

			array::clear(_position_indices);
			array::clear(_normal_indices);
			array::clear(_uv_indices);
			array::clear(_tangent_indices);
			array::clear(_binormal_indices);

			_vertex_stride = 0;
			array::clear(_vertex_buffer);
			array::clear(_index_buffer);

			aabb::reset(_aabb);
			memset(&_obb, 0, sizeof(_obb));
			memset((void*)&_layout, 0, sizeof(_layout));

			_has_normal = false;
			_has_uv = false;
		}

		void parse_indices(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, json);

			JsonArray data_json(ta);
			sjson::parse_array(data_json, obj["data"]);

			parse_index_array(_position_indices, data_json[0]);

			if (_has_normal)
			{
				parse_index_array(_normal_indices, data_json[1]);
			}
			if (_has_uv)
			{
				parse_index_array(_uv_indices, data_json[2]);
			}
		}

		void parse(const char* geometry)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, geometry);

			_has_normal = json_object::has(obj, "normal");
			_has_uv     = json_object::has(obj, "texcoord");

			parse_float_array(_positions, obj["position"]);

			if (_has_normal)
			{
				parse_float_array(_normals, obj["normal"]);
			}
			if (_has_uv)
			{
				parse_float_array(_uvs, obj["texcoord"]);
			}

			parse_indices(obj["indices"]);

			_vertex_stride = 0;
			_vertex_stride += 3 * sizeof(f32);
			_vertex_stride += (_has_normal ? 3 * sizeof(f32) : 0);
			_vertex_stride += (_has_uv     ? 2 * sizeof(f32) : 0);

			// Generate vb/ib
			array::resize(_index_buffer, array::size(_position_indices));

			u16 index = 0;
			for (u32 i = 0; i < array::size(_position_indices); ++i)
			{
				_index_buffer[i] = index++;

				const u16 p_idx = _position_indices[i] * 3;
				Vector3 xyz;
				xyz.x = _positions[p_idx + 0];
				xyz.y = _positions[p_idx + 1];
				xyz.z = _positions[p_idx + 2];
				array::push(_vertex_buffer, (char*)&xyz, sizeof(xyz));

				if (_has_normal)
				{
					const u16 n_idx = _normal_indices[i] * 3;
					Vector3 n;
					n.x = _normals[n_idx + 0];
					n.y = _normals[n_idx + 1];
					n.z = _normals[n_idx + 2];
					array::push(_vertex_buffer, (char*)&n, sizeof(n));
				}
				if (_has_uv)
				{
					const u16 t_idx = _uv_indices[i] * 2;
					Vector2 uv;
					uv.x = _uvs[t_idx + 0];
					uv.y = _uvs[t_idx + 1];
					array::push(_vertex_buffer, (char*)&uv, sizeof(uv));
				}
			}

			// Vertex layout
			_layout.begin();
			_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

			if (_has_normal)
			{
				_layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true);
			}
			if (_has_uv)
			{
				_layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			}

			_layout.end();

			// Bounds
			aabb::from_points(_aabb
				, array::size(_positions) / 3
				, sizeof(_positions[0]) * 3
				, array::begin(_positions)
				);

			_obb.tm = from_quaternion_translation(QUATERNION_IDENTITY, aabb::center(_aabb));
			_obb.half_extents = (_aabb.max - _aabb.min) * 0.5f;
		}

		void write()
		{
			BgfxWriter writer(_opts._binary_writer);
			bgfx::write(&writer, _layout);
			_opts.write(_obb);

			_opts.write(array::size(_vertex_buffer) / _vertex_stride);
			_opts.write(_vertex_stride);
			_opts.write(array::size(_index_buffer));

			_opts.write(_vertex_buffer);
			_opts.write(array::begin(_index_buffer), array::size(_index_buffer) * sizeof(u16));
		}
	};

	s32 compile_node(MeshCompiler& mc, CompileOptions& opts, const JsonObject& geometries, const HashMap<StringView, const char*>::Entry* entry)
	{
		TempAllocator4096 ta;
		const StringView key = entry->first;
		const char* node = entry->second;
		const char* geometry = geometries[key];

		const StringId32 node_name(key.data(), key.length());
		opts.write(node_name._id);

		JsonObject obj_node(ta);
		sjson::parse(obj_node, node);

		mc.reset();
		mc.parse(geometry);
		mc.write();

		if (json_object::has(obj_node, "children"))
		{
			JsonObject children(ta);
			sjson::parse_object(children, obj_node["children"]);

			auto cur = json_object::begin(children);
			auto end = json_object::end(children);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(children, cur);

				s32 err = compile_node(mc, opts, geometries, cur);
				DATA_COMPILER_ENSURE(err == 0, opts);
			}
		}

		return 0;
	}

	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, buf);

		JsonObject geometries(ta);
		sjson::parse(geometries, obj["geometries"]);
		JsonObject nodes(ta);
		sjson::parse(nodes, obj["nodes"]);

		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_MESH));
		opts.write(json_object::size(geometries));

		MeshCompiler mc(opts);

		auto cur = json_object::begin(nodes);
		auto end = json_object::end(nodes);
		for (; cur != end; ++cur)
		{
			JSON_OBJECT_SKIP_HOLE(nodes, cur);

			s32 err = compile_node(mc, opts, geometries, cur);
			DATA_COMPILER_ENSURE(err == 0, opts);
		}

		return 0;
	}

} // namespace mesh_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
