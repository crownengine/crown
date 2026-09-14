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
#   include "core/thread/scoped_mutex.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/data_compiler.h"
#   include "resource/mesh.h"
#   include "resource/mesh_fbx.h"
#   include "resource/mesh_gltf.h"
#   include "resource/mesh_obj.h"
#   include "resource/mesh_resource.h"
#   include <algorithm>
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
		array::clear(g._material_ranges);
	}

	bool has_normals(const GeometryInfo &g)
	{
		return g._normals.count != 0;
	}

	bool has_tangents(const GeometryInfo &g)
	{
		return g._tangents.count != 0;
	}

	bool has_bitangents(const GeometryInfo &g)
	{
		return g._bitangents.count != 0;
	}

	bool has_bones(const GeometryInfo &g)
	{
		return g._bones.count != 0;
	}

	bool has_uvs(const GeometryInfo &g)
	{
		return g._uvs.count != 0;
	}

	static u32 vertex_stride(const GeometryInfo &g, const Geometry &output)
	{
		u32 stride = 0;
		stride += 3 * sizeof(f32);
		stride += has_normals(g) ? sizeof(u32) : 0;
		stride += has_tangents(g) || !array::empty(output._tangents) ? sizeof(u32) : 0;
		stride += has_bitangents(g) || !array::empty(output._bitangents) ? sizeof(u32) : 0;
		stride += has_bones(g) ? 8*sizeof(f32) : 0;
		stride += has_uvs(g) ? 2*sizeof(f32) : 0;
		return stride;
	}

	static bgfx::VertexLayout vertex_layout(const GeometryInfo &g, const Geometry &output)
	{
		bgfx::VertexLayout layout;
		memset((void *)&layout, 0, sizeof(layout));

		layout.begin();
		layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

		if (has_normals(g))
			layout.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true);

		if (has_tangents(g) || !array::empty(output._tangents))
			layout.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true);

		if (has_bitangents(g) || !array::empty(output._bitangents))
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

	struct MikkUserData
	{
		const Geometry &source;
		const GeometryInfo &geometry;
		Geometry &output;
	};

	static int mikk_get_num_faces(const SMikkTSpaceContext *context)
	{
		const MikkUserData &data = *(MikkUserData *)context->m_pUserData;
		return int(data.geometry._position_indices.count / 3);
	}

	static int mikk_get_num_vertices_of_face(const SMikkTSpaceContext *, const int)
	{
		return 3;
	}

	static void mikk_get_position(const SMikkTSpaceContext *context, float fvPosOut[], const int iFace, const int iVert)
	{
		const MikkUserData &data = *(MikkUserData *)context->m_pUserData;
		const u32 corner = iFace*3 + iVert;
		const u32 index = data.geometry._position_indices.offset + corner;
		const u32 position_offset = data.geometry._positions.offset + data.source._position_indices[index] * 3;
		const f32 *position = &data.source._positions[position_offset];
		fvPosOut[0] = position[0];
		fvPosOut[1] = position[1];
		fvPosOut[2] = position[2];
	}

	static void mikk_get_normal(const SMikkTSpaceContext *context, float fvNormOut[], const int iFace, const int iVert)
	{
		const MikkUserData &data = *(MikkUserData *)context->m_pUserData;
		const u32 corner = iFace*3 + iVert;
		const u32 index = data.geometry._normal_indices.offset + corner;
		const u32 normal_offset = data.geometry._normals.offset + data.source._normal_indices[index] * 3;
		const f32 *normal = &data.source._normals[normal_offset];
		fvNormOut[0] = normal[0];
		fvNormOut[1] = normal[1];
		fvNormOut[2] = normal[2];
	}

	static void mikk_get_texcoord(const SMikkTSpaceContext *context, float fvTexcOut[], const int iFace, const int iVert)
	{
		const MikkUserData &data = *(MikkUserData *)context->m_pUserData;
		const u32 corner = iFace*3 + iVert;
		const u32 index = data.geometry._uv_indices.offset + corner;
		const u32 uv_offset = data.geometry._uvs.offset + data.source._uv_indices[index] * 2;
		const f32 *uv = &data.source._uvs[uv_offset];
		fvTexcOut[0] = uv[0];
		fvTexcOut[1] = 1.0f - uv[1];
	}

	static void mikk_set_tspace_basic(const SMikkTSpaceContext *context, const float fvTangent[], const float fSign, const int iFace, const int iVert)
	{
		MikkUserData &data = *(MikkUserData *)context->m_pUserData;
		const u32 corner = iFace*3 + iVert;
		const u32 index = data.geometry._normal_indices.offset + corner;
		const u32 normal_offset = data.geometry._normals.offset + data.source._normal_indices[index] * 3;
		const f32 *normal = &data.source._normals[normal_offset];
		f32 *tangent = &data.output._tangents[corner * 3];
		f32 *bitangent = &data.output._bitangents[corner * 3];

		tangent[0] = fvTangent[0];
		tangent[1] = fvTangent[1];
		tangent[2] = fvTangent[2];
		bitangent[0] = (normal[1]*fvTangent[2] - normal[2]*fvTangent[1]) * fSign;
		bitangent[1] = (normal[2]*fvTangent[0] - normal[0]*fvTangent[2]) * fSign;
		bitangent[2] = (normal[0]*fvTangent[1] - normal[1]*fvTangent[0]) * fSign;
	}

	static void generate_tangent_space(const Geometry &source, const GeometryInfo &geometry, Geometry &output)
	{
		if (!has_normals(geometry) || !has_uvs(geometry))
			return;

		const u32 num_indices = geometry._position_indices.count;
		array::resize(output._tangents, num_indices * 3);
		array::resize(output._bitangents, num_indices * 3);

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
		MikkUserData data = { source, geometry, output };
		SMikkTSpaceContext context = { &iface, &data };
		genTangSpaceDefault(&context);
	}

	static s32 generate_vertex_and_index_buffers(const Geometry &source
		, const GeometryInfo &geometry
		, Geometry &output
		, CompileOptions &opts
		)
	{
		TempAllocator512 ta;
		Buffer vertex(ta);
		HashMap<VertexKey, u32, VertexKeyHash, VertexKeyEqual> vertex_map(default_allocator());

		const u32 num_indices = geometry._position_indices.count;
		const u32 stride = vertex_stride(geometry, output);
		array::reserve(output._vertex_buffer, num_indices * stride + 1);
		array::reserve(output._index_buffer, num_indices);

		for (u32 i = 0; i < num_indices; ++i) {
			array::clear(vertex);

			const u32 position_index = geometry._position_indices.offset + i;
			const u32 idx = geometry._positions.offset + source._position_indices[position_index] * 3;
			Vector3 v;
			v.x = source._positions[idx + 0];
			v.y = source._positions[idx + 1];
			v.z = source._positions[idx + 2];
			array::push(vertex, (char *)&v, sizeof(v));

			if (has_normals(geometry)) {
				const u32 index = geometry._normal_indices.offset + i;
				const u32 idx = geometry._normals.offset + source._normal_indices[index] * 3;
				Vector3 v;
				v.x = source._normals[idx + 0];
				v.y = source._normals[idx + 1];
				v.z = source._normals[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (!array::empty(output._tangents)) {
				const u32 idx = i * 3;
				Vector3 v;
				CE_ENSURE(idx < array::size(output._tangents));
				v.x = output._tangents[idx + 0];
				v.y = output._tangents[idx + 1];
				v.z = output._tangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			} else if (has_tangents(geometry)) {
				const u32 index = geometry._tangent_indices.offset + i;
				const u32 idx = geometry._tangents.offset + source._tangent_indices[index] * 3;
				Vector3 v;
				CE_ENSURE(idx < geometry._tangents.offset + geometry._tangents.count);
				v.x = source._tangents[idx + 0];
				v.y = source._tangents[idx + 1];
				v.z = source._tangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (!array::empty(output._bitangents)) {
				const u32 idx = i * 3;
				CE_ENSURE(idx < array::size(output._bitangents));
				Vector3 v;
				v.x = output._bitangents[idx + 0];
				v.y = output._bitangents[idx + 1];
				v.z = output._bitangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			} else if (has_bitangents(geometry)) {
				const u32 index = geometry._bitangent_indices.offset + i;
				const u32 idx = geometry._bitangents.offset + source._bitangent_indices[index] * 3;
				CE_ENSURE(idx < geometry._bitangents.offset + geometry._bitangents.count);
				Vector3 v;
				v.x = source._bitangents[idx + 0];
				v.y = source._bitangents[idx + 1];
				v.z = source._bitangents[idx + 2];
				u32 vu = to_uint(v);
				array::push(vertex, (char *)&vu, sizeof(vu));
			}

			if (has_bones(geometry)) {
				const u32 bone_index = geometry._bone_indices.offset + i;
				const u32 bidx = geometry._bones.offset + source._bone_indices[bone_index] * 4;
				Vector4 b;
				b.x = source._bones[bidx + 0];
				b.y = source._bones[bidx + 1];
				b.z = source._bones[bidx + 2];
				b.w = source._bones[bidx + 3];
				array::push(vertex, (char *)&b, sizeof(b));

				const u32 weight_index = geometry._weight_indices.offset + i;
				const u32 widx = geometry._weights.offset + source._weight_indices[weight_index] * 4;
				Vector4 w;
				w.x = source._weights[widx + 0];
				w.y = source._weights[widx + 1];
				w.z = source._weights[widx + 2];
				w.w = source._weights[widx + 3];
				array::push(vertex, (char *)&w, sizeof(w));
			}

			if (has_uvs(geometry)) {
				const u32 index = geometry._uv_indices.offset + i;
				const u32 idx = geometry._uvs.offset + source._uv_indices[index] * 2;
				CE_ENSURE(idx < geometry._uvs.offset + geometry._uvs.count);
				Vector2 v;
				v.x = source._uvs[idx + 0];
				v.y = source._uvs[idx + 1];
				array::push(vertex, (char *)&v, sizeof(v));
			}

			const u32 vertex_size = array::size(vertex);
			CE_ENSURE(vertex_size == stride);

			const u32 INVALID_VERTEX_INDEX = UINT32_MAX;
			VertexKey key = { array::begin(vertex), vertex_size };
			u32 index = hash_map::get(vertex_map, key, INVALID_VERTEX_INDEX);

			if (index == INVALID_VERTEX_INDEX) {
				index = array::size(output._vertex_buffer) / vertex_size;
				RETURN_IF_FALSE(MESH, index <= UINT16_MAX
					, opts
					, "Mesh has too many vertices: %u (max %u)"
					, index + 1
					, UINT16_MAX + 1u
					);

				const u32 vertex_offset = array::size(output._vertex_buffer);
				array::push(output._vertex_buffer, array::begin(vertex), vertex_size);
				VertexKey stored_key = { array::begin(output._vertex_buffer) + vertex_offset, vertex_size };
				hash_map::set(vertex_map, stored_key, index);
			}

			array::push_back(output._index_buffer, (u16)index);
		}

		return 0;
	}

	static void merge_material_ranges(Geometry &g)
	{
		bool has_duplicate_slots = false;
		for (u32 i = 0; i < array::size(g._material_ranges); ++i) {
			for (u32 j = 0; j < i; ++j) {
				if (g._material_ranges[i].slot == g._material_ranges[j].slot) {
					has_duplicate_slots = true;
					break;
				}
			}
			if (has_duplicate_slots)
				break;
		}
		if (!has_duplicate_slots)
			return;

		const u32 num_ranges = array::size(g._material_ranges);
		u32 write = 0;
		u32 i = 0;
		while (i < num_ranges) {
			const StringId32 slot = g._material_ranges[i].slot;
			const u32 index_offset = g._material_ranges[i].index_offset;
			u32 num_indices = g._material_ranges[i].num_indices;
			u32 group_end = i + 1;
			for (;;) {
				u32 j = group_end;
				while (j < num_ranges && g._material_ranges[j].slot != slot)
					++j;
				if (j == num_ranges)
					break;

				const MeshMaterialRange range = g._material_ranges[j];
				const u32 insertion_offset = g._material_ranges[group_end - 1].index_offset
					+ g._material_ranges[group_end - 1].num_indices;
				std::rotate(array::begin(g._index_buffer) + insertion_offset
					, array::begin(g._index_buffer) + range.index_offset
					, array::begin(g._index_buffer) + range.index_offset + range.num_indices
					);

				for (u32 k = j; k > group_end; --k) {
					g._material_ranges[k] = g._material_ranges[k - 1];
					g._material_ranges[k].index_offset += range.num_indices;
				}
				g._material_ranges[group_end] = range;
				g._material_ranges[group_end].index_offset = insertion_offset;
				num_indices += range.num_indices;
				++group_end;
			}
			g._material_ranges[write++] = { slot, index_offset, num_indices };
			i = group_end;
		}
		array::resize(g._material_ranges, write);
	}

	static OBB obb(const Geometry &g, const GeometryInfo &geometry)
	{
		AABB aabb;
		OBB obb;
		aabb::reset(aabb);
		memset(&obb, 0, sizeof(obb));

		if (geometry._positions.count != 0) {
			aabb::from_points(aabb
				, geometry._positions.count / 3
				, sizeof(g._positions[0]) * 3
				, array::begin(g._positions) + geometry._positions.offset
				);
		}

		obb.tm = from_quaternion_translation(QUATERNION_IDENTITY, aabb::center(aabb));
		obb.half_extents = (aabb.max - aabb.min) * 0.5f;
		return obb;
	}

	// Finds the tightest bounding sphere by calling add_points() multiple times on the same
	// randomly ordered positions. Uses a seed dependent on initial positions to guarantee stable
	// results.
	static Sphere sphere(const Geometry &g, const GeometryInfo &geometry)
	{
		const u32 MAX_TRIES = 256;
		Sphere sphere;
		sphere::reset(sphere);

		if (geometry._positions.count != 0) {
			const f32 *source_positions = array::begin(g._positions) + geometry._positions.offset;
			const u16 seed = (u16)murmur64(source_positions
				, geometry._positions.count*sizeof(g._positions[0])
				, 0u
				);
			Random random((s32)seed);

			Array<u32> indices(default_allocator());
			array::resize(indices, geometry._positions.count / 3);

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

				sphere::add_points(s
					, array::size(indices)
					, sizeof(g._positions[0]) * 3
					, source_positions
					, array::begin(indices)
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
		Buffer settings_buf = opts.read_optional(importer_settings.c_str());
		JsonObject settings(ta);
		RETURN_IF_ERROR(sjson::parse(settings, settings_buf));

		if (json_object::has(settings, "tangents")) {
			DynamicString tangents(ta);
			RETURN_IF_ERROR(sjson::parse_string(tangents, settings["tangents"]));
			calculate_tangents = tangents == "calculate";
		}

		Geometry geo(default_allocator());

		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_MESH));
		opts.write(hash_map::size(m._geometries));

		auto cur = hash_map::begin(m._geometries);
		auto end = hash_map::end(m._geometries);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(m._geometries, cur);

			{
				u32 num_geo_names = 0;

				auto cur = hash_map::begin(m._nodes);
				auto end = hash_map::end(m._nodes);
				for (; cur != end; ++cur) {
					HASH_MAP_SKIP_HOLE(m._nodes, cur);

					if (cur->second._geometry == cur->first)
						++num_geo_names;
				}

				opts.write(num_geo_names);
			}

			{
				auto cur = hash_map::begin(m._nodes);
				auto end = hash_map::end(m._nodes);
				for (; cur != end; ++cur) {
					HASH_MAP_SKIP_HOLE(m._nodes, cur);

					if (cur->second._geometry == cur->first)
						opts.write(cur->second._geometry.to_string_id()._id);
				}
			}

			mesh::reset(geo);
			const GeometryInfo &geometry_info = cur->second;
			if (geometry_info._material_ranges.count != 0)
				array::push(geo._material_ranges, array::begin(m._geometry._material_ranges) + geometry_info._material_ranges.offset, geometry_info._material_ranges.count);

			if (calculate_tangents)
				generate_tangent_space(m._geometry, geometry_info, geo);
			ENSURE_OR_RETURN(MESH, mesh::generate_vertex_and_index_buffers(m._geometry, geometry_info, geo, opts) == 0, opts);

			if (array::empty(geo._material_ranges))
				array::push_back(geo._material_ranges, { STRING_ID_32("default", UINT32_C(0x5974b5ec)), 0, array::size(geo._index_buffer) });
			else
				mesh::merge_material_ranges(geo);

			bgfx::VertexLayout layout = mesh::vertex_layout(geometry_info, geo);
			u32 stride = mesh::vertex_stride(geometry_info, geo);

			BgfxWriter writer(opts._binary_writer);
			bgfx::write(&writer, layout);
			opts.write(mesh::obb(m._geometry, geometry_info));
			opts.write(mesh::sphere(m._geometry, geometry_info));

			opts.write(array::size(geo._vertex_buffer) / stride);
			opts.write(stride);
			opts.write(array::size(geo._index_buffer));

			opts.write(array::size(geo._material_ranges));
			for (u32 i = 0; i < array::size(geo._material_ranges); ++i) {
				const MeshMaterialRange &range = geo._material_ranges[i];
				opts.write(range.slot);
				opts.write(range.index_offset);
				opts.write(range.num_indices);
			}

			opts.write(geo._vertex_buffer);
			opts.write(array::begin(geo._index_buffer), array::size(geo._index_buffer) * sizeof(u16));
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
			if (str_has_suffix_case(source.c_str(), ".gltf")
				|| str_has_suffix_case(source.c_str(), ".glb")
				) {
				gltf::MeshImportOptions import_options = {};
				if (json_object::has(obj, "rigid_skinning")) {
					import_options.rigid_skinning = RETURN_IF_ERROR(sjson::parse_bool(obj["rigid_skinning"]));
				}
				return crown::gltf::parse(m, source.c_str(), import_options, opts);
			}

			Buffer fbx_buf = opts.read(source.c_str());
			return fbx::parse(m, fbx_buf, opts);
		} else {
			return mesh::parse(m, buf, opts);
		}
	}

	s32 parse(const Mesh **m, const char *path, CompileOptions &opts)
	{
		MeshCache *cache = (MeshCache *)opts._data_compiler.user_data(RESOURCE_TYPE_MESH);
		CE_ENSURE(cache != NULL);

		s32 err = 0;
		StringId64 path_id(path);
		opts.fake_read(path);
		Mesh *mesh = mesh_cache::get(*cache, path);
		if (mesh == NULL) {
			mesh = CE_NEW(default_allocator(), Mesh)(default_allocator());
			RETURN_IF_FILE_MISSING(MESH, path, opts);
			Buffer buf = opts.read(path);
			err = parse_internal(*mesh, buf, opts);
			ENSURE_OR_RETURN(MESH, err == 0, opts);
			mesh->_path = path_id;
			Mesh *cached_mesh = mesh_cache::add(*cache, mesh);
			if (cached_mesh != mesh) {
				CE_DELETE(default_allocator(), mesh);
				mesh = cached_mesh;
			}
		}

		*m = mesh;
		return err;
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
	, _material_ranges(a)
{
	mesh::reset(*this);
}

Mesh::Mesh(Allocator &a)
	: _geometry(a)
	, _geometries(a)
	, _nodes(a)
{
	_cache_node.next = NULL;
	_cache_node.prev = NULL;
}

namespace mesh_cache
{
	Mesh *get(MeshCache &cache, const char *path)
	{
		ScopedMutex sm(cache._mutex);
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

	Mesh *add(MeshCache &cache, Mesh *mesh)
	{
		ScopedMutex sm(cache._mutex);

		ListNode *cur;
		list_for_each(cur, &cache._meshes)
		{
			Mesh *cached_mesh = (Mesh *)container_of(cur, Mesh, _cache_node);

			if (cached_mesh->_path == mesh->_path)
				return cached_mesh;
		}

		list::add(mesh->_cache_node, cache._meshes);
		return mesh;
	}

	void clear(MeshCache &cache)
	{
		ScopedMutex sm(cache._mutex);

		ListNode *cur;
		ListNode *tmp;
		list_for_each_safe(cur, tmp, &cache._meshes)
		{
			Mesh *mesh = (Mesh *)container_of(cur, Mesh, _cache_node);
			CE_DELETE(default_allocator(), mesh);
		}
		list::init_head(cache._meshes);
	}

} // namespace mesh_cache

MeshCache::MeshCache()
{
	list::init_head(_meshes);
}

MeshCache::~MeshCache()
{
	mesh_cache::clear(*this);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
