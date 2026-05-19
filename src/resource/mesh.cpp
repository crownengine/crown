/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/containers/vector.inl"
#   include "core/filesystem/filesystem.h"
#   include "core/json/json_object.inl"
#   include "core/json/sjson.h"
#   include "core/list.inl"
#   include "core/math/aabb.inl"
#   include "core/math/constants.h"
#   include "core/math/matrix4x4.inl"
#   include "core/math/random.inl"
#   include "core/math/sphere.inl"
#   include "core/math/vector2.inl"
#   include "core/math/vector3.inl"
#   include "core/memory/temp_allocator.inl"
#   include "core/murmur.h"
#   include "core/strings/dynamic_string.inl"
#   include "core/strings/string.inl"
#   include "core/strings/string_id.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/data_compiler.h"
#   include "resource/mesh.h"
#   include "resource/mesh_fbx.h"
#   include "resource/mesh_obj.h"
#   include "resource/mesh_resource.h"
#   include <bx/error.h>
#   include <bx/readerwriter.h>
#   include <mikktspace.h>
#   include <vertexlayout.h> // bgfx::write, bgfx::read

LOG_SYSTEM(MESH, "mesh")

namespace crown
{
namespace mesh
{
	/// Writer interface.
	struct BgfxWriter : public bx::WriterI
	{
		BinaryWriter *_bw;

		///
		explicit BgfxWriter(BinaryWriter &bw)
			: _bw(&bw)
		{
		}

		///
		virtual ~BgfxWriter()
		{
		}

		///
		virtual int32_t write(const void *_data, int32_t _size, bx::Error *_err)
		{
			CE_UNUSED(_err);
			_bw->write(_data, _size);
			return _size; // FIXME: return the actual number of bytes written
		}
	};

	static void reset(Geometry &g)
	{
		array::clear(g._positions);
		array::clear(g._normals);
		array::clear(g._tangents);
		array::clear(g._bitangents);
		array::clear(g._bones);
		array::clear(g._weights);
		array::clear(g._uvs);

		array::clear(g._position_indices);
		array::clear(g._normal_indices);
		array::clear(g._tangent_indices);
		array::clear(g._bitangent_indices);
		array::clear(g._bone_indices);
		array::clear(g._weight_indices);
		array::clear(g._uv_indices);

		array::clear(g._vertex_buffer);
		array::clear(g._index_buffer);
	}

	bool has_normals(Geometry &g)
	{
		return array::size(g._normals) != 0;
	}

	bool has_tangents(Geometry &g)
	{
		return array::size(g._tangents) != 0;
	}

	bool has_bitangents(Geometry &g)
	{
		return array::size(g._bitangents) != 0;
	}

	bool has_bones(Geometry &g)
	{
		return array::size(g._bones) != 0;
	}

	bool has_uvs(Geometry &g)
	{
		return array::size(g._uvs) != 0;
	}

	static u32 vertex_stride(Geometry &g)
	{
		u32 stride = 0;
		stride += 3 * sizeof(f32);
		stride += has_normals(g) ? sizeof(u32) : 0;
		stride += has_tangents(g) ? sizeof(u32) : 0;
		stride += has_bitangents(g) ? sizeof(u32) : 0;
		stride += has_bones(g) ? 8*sizeof(f32) : 0;
		stride += has_uvs(g) ? 2*sizeof(f32) : 0;
		return stride;
	}

	static bgfx::VertexLayout vertex_layout(Geometry &g)
	{
		bgfx::VertexLayout layout;
		memset((void *)&layout, 0, sizeof(layout));

		layout.begin();
		layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

		if (has_normals(g))
			layout.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true);

		if (has_tangents(g))
			layout.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true);

		if (has_bitangents(g))
			layout.add(bgfx::Attrib::Bitangent, 4, bgfx::AttribType::Uint8, true, true);

		if (has_bones(g)) {
			layout.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Float);
			layout.add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float);
		}

		if (has_uvs(g))
			layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);

		layout.end();
		return layout;
	}

	static u32 to_unorm(f32 val, f32 scale)
	{
		return u32(fround(clamp(val, 0.0f, 1.0f) * scale));
	}

	static u32 to_uint(const Vector3 normal)
	{
		u32 n;
		n  = to_unorm(normal.x * 0.5f + 0.5f, 255.0f) <<  0;
		n |= to_unorm(normal.y * 0.5f + 0.5f, 255.0f) <<  8;
		n |= to_unorm(normal.z * 0.5f + 0.5f, 255.0f) << 16;
		n |= to_unorm(0.0f     * 0.5f + 0.5f, 255.0f) << 24;
		return n;
	}

	struct VertexKey
	{
		const char *data;
		u32 size;
	};

	struct VertexKeyHash
	{
		u32 operator()(const VertexKey &key) const
		{
			return u32(murmur64(key.data, key.size, 0u));
		}
	};

	struct VertexKeyEqual
	{
		bool operator()(const VertexKey &a, const VertexKey &b) const
		{
			return a.size == b.size
				&& memcmp(a.data, b.data, a.size) == 0
				;
		}
	};

	static int mikk_get_num_faces(const SMikkTSpaceContext *context)
	{
		const Geometry &g = *(Geometry *)context->m_pUserData;
		return int(array::size(g._position_indices) / 3);
	}

	static int mikk_get_num_vertices_of_face(const SMikkTSpaceContext *, const int)
	{
		return 3;
	}

	static void mikk_get_position(const SMikkTSpaceContext *context, float fvPosOut[], const int iFace, const int iVert)
	{
		const Geometry &g = *(Geometry *)context->m_pUserData;
		const f32 *position = &g._positions[g._position_indices[iFace*3 + iVert] * 3];
		fvPosOut[0] = position[0];
		fvPosOut[1] = position[1];
		fvPosOut[2] = position[2];
	}

	static void mikk_get_normal(const SMikkTSpaceContext *context, float fvNormOut[], const int iFace, const int iVert)
	{
		const Geometry &g = *(Geometry *)context->m_pUserData;
		const f32 *normal = &g._normals[g._normal_indices[iFace*3 + iVert] * 3];
		fvNormOut[0] = normal[0];
		fvNormOut[1] = normal[1];
		fvNormOut[2] = normal[2];
	}

	static void mikk_get_texcoord(const SMikkTSpaceContext *context, float fvTexcOut[], const int iFace, const int iVert)
	{
		const Geometry &g = *(Geometry *)context->m_pUserData;
		const f32 *uv = &g._uvs[g._uv_indices[iFace*3 + iVert] * 2];
		fvTexcOut[0] = uv[0];
		fvTexcOut[1] = 1.0f - uv[1];
	}

	static void mikk_set_tspace_basic(const SMikkTSpaceContext *context, const float fvTangent[], const float fSign, const int iFace, const int iVert)
	{
		Geometry &g = *(Geometry *)context->m_pUserData;
		const u32 corner = iFace*3 + iVert;
		const f32 *normal = &g._normals[g._normal_indices[corner] * 3];
		f32 *tangent = &g._tangents[corner * 3];
		f32 *bitangent = &g._bitangents[corner * 3];

		tangent[0] = fvTangent[0];
		tangent[1] = fvTangent[1];
		tangent[2] = fvTangent[2];
		bitangent[0] = (normal[1]*fvTangent[2] - normal[2]*fvTangent[1]) * fSign;
		bitangent[1] = (normal[2]*fvTangent[0] - normal[0]*fvTangent[2]) * fSign;
		bitangent[2] = (normal[0]*fvTangent[1] - normal[1]*fvTangent[0]) * fSign;
	}

	static void generate_tangent_space(Geometry &g)
	{
		if (!has_normals(g) || !has_uvs(g))
			return;

		const u32 num_indices = array::size(g._position_indices);
		array::resize(g._tangents, num_indices * 3);
		array::resize(g._bitangents, num_indices * 3);
		array::resize(g._tangent_indices, num_indices);
		array::resize(g._bitangent_indices, num_indices);

		for (u32 i = 0; i < num_indices; ++i)
			g._tangent_indices[i] = g._bitangent_indices[i] = i;

		SMikkTSpaceInterface iface =
		{
			mikk_get_num_faces,
			mikk_get_num_vertices_of_face,
			mikk_get_position,
			mikk_get_normal,
			mikk_get_texcoord,
			mikk_set_tspace_basic,
			0
		};
		SMikkTSpaceContext context = { &iface, &g };
		genTangSpaceDefault(&context);
	}

	static s32 generate_vertex_and_index_buffers(Geometry &g, CompileOptions &opts)
	{
		TempAllocator512 ta;
		Buffer vertex(ta);
		HashMap<VertexKey, u32, VertexKeyHash, VertexKeyEqual> vertex_map(default_allocator());

		const u32 num_indices = array::size(g._position_indices);
		const u32 stride = vertex_stride(g);
		array::reserve(g._vertex_buffer, num_indices * stride + 1);
		array::reserve(g._index_buffer, num_indices);

		for (u32 i = 0; i < num_indices; ++i) {
			array::clear(vertex);

			const u32 idx = g._position_indices[i] * 3;
			Vector3 v;
			v.x = g._positions[idx + 0];
			v.y = g._positions[idx + 1];
			v.z = g._positions[idx + 2];
			array::push(vertex, (char *)&v, sizeof(v));

			if (has_normals(g)) {
				const u32 idx = g._normal_indices[i] * 3;
				Vector3 v;
				v.x = g._normals[idx + 0];
				v.y = g._normals[idx + 1];
				v.z = g._normals[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (has_tangents(g)) {
				const u32 idx = g._tangent_indices[i] * 3;
				Vector3 v;
				CE_ENSURE(idx < array::size(g._tangents));
				v.x = g._tangents[idx + 0];
				v.y = g._tangents[idx + 1];
				v.z = g._tangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (has_bitangents(g)) {
				const u32 idx = g._bitangent_indices[i] * 3;
				CE_ENSURE(idx < array::size(g._bitangents));
				Vector3 v;
				v.x = g._bitangents[idx + 0];
				v.y = g._bitangents[idx + 1];
				v.z = g._bitangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (has_bones(g)) {
				const u32 bidx = g._bone_indices[i] * 4;
				Vector4 b;
				b.x = g._bones[bidx + 0];
				b.y = g._bones[bidx + 1];
				b.z = g._bones[bidx + 2];
				b.w = g._bones[bidx + 3];
				array::push(vertex, (char *)&b, sizeof(b));

				const u32 widx = g._weight_indices[i] * 4;
				Vector4 w;
				w.x = g._weights[widx + 0];
				w.y = g._weights[widx + 1];
				w.z = g._weights[widx + 2];
				w.w = g._weights[widx + 3];
				array::push(vertex, (char *)&w, sizeof(w));
			}

			if (has_uvs(g)) {
				const u32 idx = g._uv_indices[i] * 2;
				CE_ENSURE(idx < array::size(g._uvs));
				Vector2 v;
				v.x = g._uvs[idx + 0];
				v.y = g._uvs[idx + 1];
				array::push(vertex, (char *)&v, sizeof(v));
			}

			const u32 vertex_size = array::size(vertex);
			CE_ENSURE(vertex_size == stride);

			const u32 INVALID_VERTEX_INDEX = UINT32_MAX;
			VertexKey key = { array::begin(vertex), vertex_size };
			u32 index = hash_map::get(vertex_map, key, INVALID_VERTEX_INDEX);

			if (index == INVALID_VERTEX_INDEX) {
				index = array::size(g._vertex_buffer) / vertex_size;
				RETURN_IF_FALSE(MESH, index <= UINT16_MAX
					, opts
					, "Mesh has too many vertices: %u (max %u)"
					, index + 1
					, UINT16_MAX + 1u
					);

				const u32 vertex_offset = array::size(g._vertex_buffer);
				array::push(g._vertex_buffer, array::begin(vertex), vertex_size);
				VertexKey stored_key = { array::begin(g._vertex_buffer) + vertex_offset, vertex_size };
				hash_map::set(vertex_map, stored_key, index);
			}

			array::push_back(g._index_buffer, (u16)index);
		}

		return 0;
	}

	static void geometry_names(Vector<DynamicString> &names, const Mesh &m, const DynamicString &geometry)
	{
		auto cur = hash_map::begin(m._nodes);
		auto end = hash_map::end(m._nodes);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(m._nodes, cur);

			if (cur->second._geometry == geometry)
				vector::push_back(names, cur->first);
		}
	}

	static OBB obb(Geometry &g)
	{
		AABB aabb;
		OBB obb;
		aabb::reset(aabb);
		memset(&obb, 0, sizeof(obb));

		if (array::size(g._positions) != 0) {
			aabb::from_points(aabb
				, array::size(g._positions) / 3
				, sizeof(g._positions[0]) * 3
				, array::begin(g._positions)
				);
		}

		obb.tm = from_quaternion_translation(QUATERNION_IDENTITY, aabb::center(aabb));
		obb.half_extents = (aabb.max - aabb.min) * 0.5f;
		return obb;
	}

	// Finds the tightest bounding sphere by calling add_points() multiple times on the same
	// randomly ordered positions. Uses a seed dependent on initial positions to guarantee stable
	// results.
	static Sphere sphere(Geometry &g)
	{
		const u32 MAX_TRIES = 256;
		Sphere sphere;
		sphere::reset(sphere);

		if (array::size(g._positions) != 0) {
			const u16 seed = (u16)murmur64(array::begin(g._positions)
				, array::size(g._positions)*sizeof(g._positions[0])
				, 0u
				);
			Random random((s32)seed);

			Array<f32> positions(default_allocator());
			Array<u32> indices(default_allocator());
			array::resize(positions, array::size(g._positions));
			array::resize(indices, array::size(g._positions) / 3);

			for (u32 j = 0; j < array::size(indices); ++j)
				indices[j] = j;

			Sphere s;
			for (u32 i = 0; i < MAX_TRIES; ++i) {
				sphere::reset(s);

				// Shuffle index.
				for (u32 i = 0; i < array::size(indices); ++i) {
					s32 k = random.integer(array::size(indices));
					exchange(indices[i], indices[k]);
				}

				// TODO: just add a sphere::add_points() that supports index buffers.
				for (u32 i = 0; i < array::size(indices); ++i) {
					positions[i*3 + 0] = g._positions[indices[i]*3 + 0];
					positions[i*3 + 1] = g._positions[indices[i]*3 + 1];
					positions[i*3 + 2] = g._positions[indices[i]*3 + 2];
				}

				sphere::add_points(s
					, array::size(g._positions) / 3
					, sizeof(g._positions[0]) * 3
					, array::begin(positions)
					);

				if (sphere::volume(s) < sphere::volume(sphere) || i == 0)
					sphere = s;
			}
		}

		return sphere;
	}

	s32 write(Mesh &m, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		bool calculate_tangents = true;
		DynamicString importer_settings(ta);
		importer_settings.set(opts.source_path(), u32(strrchr(opts.source_path(), '.') - opts.source_path()));
		importer_settings += ".importer_settings";
		Buffer settings_buf = opts.read(importer_settings.c_str());
		JsonObject settings(ta);
		RETURN_IF_ERROR(sjson::parse(settings, settings_buf));

		if (json_object::has(settings, "tangents")) {
			DynamicString tangents(ta);
			RETURN_IF_ERROR(sjson::parse_string(tangents, settings["tangents"]));
			calculate_tangents = tangents == "calculate";
		}

		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_MESH));
		opts.write(hash_map::size(m._geometries));

		auto cur = hash_map::begin(m._geometries);
		auto end = hash_map::end(m._geometries);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(m._geometries, cur);

			Vector<DynamicString> geo_names(default_allocator());
			geometry_names(geo_names, m, cur->first);
			u32 num_geo_names = vector::size(geo_names);

			opts.write(num_geo_names);
			for (u32 i = 0; i < num_geo_names; ++i)
				opts.write(geo_names[i].to_string_id()._id);

			Geometry *geo = (Geometry *)&cur->second;
			if (calculate_tangents)
				generate_tangent_space(*geo);
			ENSURE_OR_RETURN(MESH, mesh::generate_vertex_and_index_buffers(*geo, opts) == 0, opts);

			bgfx::VertexLayout layout = mesh::vertex_layout(*geo);
			u32 stride = mesh::vertex_stride(*geo);

			BgfxWriter writer(opts._binary_writer);
			bgfx::write(&writer, layout);
			opts.write(mesh::obb(*geo));
			opts.write(mesh::sphere(*geo));

			opts.write(array::size(geo->_vertex_buffer) / stride);
			opts.write(stride);
			opts.write(array::size(geo->_index_buffer));

			opts.write(geo->_vertex_buffer);
			opts.write(array::begin(geo->_index_buffer), array::size(geo->_index_buffer) * sizeof(u16));
		}

		return 0;
	}

	static s32 parse_internal(Mesh &m, Buffer &buf, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf));

		DynamicString source(ta);
		if (json_object::has(obj, "source")) {
			RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]));

			RETURN_IF_FILE_MISSING(MESH, source.c_str(), opts);
			if (str_has_suffix_case(source.c_str(), ".obj"))
				return crown::obj::parse(m, source.c_str(), opts);

			Buffer fbx_buf = opts.read(source.c_str());
			return fbx::parse(m, fbx_buf, opts);
		} else {
			return mesh::parse(m, buf, opts);
		}
	}

	s32 parse(Mesh &m, const char *path, CompileOptions &opts)
	{
		MeshCache *cache = (MeshCache *)opts._data_compiler.user_data(RESOURCE_TYPE_MESH);

		if (cache == NULL) {
			RETURN_IF_FILE_MISSING(MESH, path, opts);
			Buffer buf = opts.read(path);
			return parse_internal(m, buf, opts);
		} else {
			s32 err = 0;
			StringId64 path_id(path);
			Mesh *mesh = mesh_cache::get(*cache, path);
			if (mesh == NULL) {
				mesh = CE_NEW(default_allocator(), Mesh)(default_allocator());
				RETURN_IF_FILE_MISSING(MESH, path, opts);
				Buffer buf = opts.read(path);
				err = parse_internal(*mesh, buf, opts);
				ENSURE_OR_RETURN(MESH, err == 0, opts);
				mesh->_path = path_id;
				mesh_cache::add(*cache, mesh);
			}

			m = *mesh;
			return err;
		}
	}

	s32 parse(Mesh &m, CompileOptions &opts)
	{
		Buffer buf = opts.read();
		return parse_internal(m, buf, opts);
	}

} // namespace mesh

Node::Node(Allocator &a)
	: _local_pose(MATRIX4X4_IDENTITY)
	, _geometry(a)
{
}

Geometry::Geometry(Allocator &a)
	: _positions(a)
	, _normals(a)
	, _uvs(a)
	, _tangents(a)
	, _bitangents(a)
	, _bones(a)
	, _weights(a)
	, _position_indices(a)
	, _normal_indices(a)
	, _tangent_indices(a)
	, _bitangent_indices(a)
	, _bone_indices(a)
	, _weight_indices(a)
	, _uv_indices(a)
	, _vertex_buffer(a)
	, _index_buffer(a)
{
	mesh::reset(*this);
}

Mesh::Mesh(Allocator &a)
	: _geometries(a)
	, _nodes(a)
{
	_cache_node.next = NULL;
	_cache_node.prev = NULL;
}

namespace mesh_cache
{
	Mesh *get(MeshCache &cache, const char *path)
	{
		StringId64 path_id(path);

		ListNode *cur;
		list_for_each(cur, &cache._meshes)
		{
			Mesh *mesh = (Mesh *)container_of(cur, Mesh, _cache_node);

			if (mesh->_path == path_id)
				return mesh;
		}

		return NULL;
	}

	void add(MeshCache &cache, Mesh *mesh)
	{
		list::add(mesh->_cache_node, cache._meshes);
	}

} // namespace mesh_cache

MeshCache::MeshCache()
{
	list::init_head(_meshes);
}

MeshCache::~MeshCache()
{
	// Destroy meshes.
	ListNode *cur;
	ListNode *tmp;
	list_for_each_safe(cur, tmp, &_meshes)
	{
		Mesh *m = (Mesh *)container_of(cur, Mesh, _cache_node);
		CE_DELETE(default_allocator(), m);
	}
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
