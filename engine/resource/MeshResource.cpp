/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DynamicString.h"
#include "Filesystem.h"
#include "JSONParser.h"
#include "Log.h"
#include "MeshResource.h"
#include "TempAllocator.h"
#include "Vector3.h"

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
		return position == other.position &&
				normal == other.normal &&
				texcoord == other.texcoord;
	}
};

MeshHeader			m_mesh_header;
bool				m_has_normal;
bool				m_has_texcoord;

List<MeshVertex>	m_vertices(default_allocator());
List<uint16_t>		m_indices(default_allocator());

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	// Read data arrays
	JSONElement position = root.key_or_nil("position");
	JSONElement normal = root.key_or_nil("normal");
	JSONElement texcoord = root.key_or_nil("texcoord");

	m_has_normal = false;
	m_has_texcoord = false;

	if (position.is_nil())
	{
		Log::e("Bad mesh: array 'position' not found.");
		return;
	}
	List<float> position_array(default_allocator());
	position.to_array(position_array);


	List<float> normal_array(default_allocator());
	if (!normal.is_nil())
	{
		m_has_normal = true;
		normal.to_array(normal_array);
	}

	List<float> texcoord_array(default_allocator());
	if (!texcoord.is_nil())
	{
		m_has_texcoord = true;
		texcoord.to_array(texcoord_array);
	}

	// Read index arrays
	JSONElement index = root.key_or_nil("index");
	if (index.is_nil())
	{
		Log::e("Bad mesh: array 'index' not found.");
		return;
	}

	List<uint16_t> position_index(default_allocator());
	List<uint16_t> normal_index(default_allocator());
	List<uint16_t> texcoord_index(default_allocator());

	index[0].to_array(position_index);

	if (m_has_normal)
	{
		index[1].to_array(normal_index);
	}

	if (m_has_texcoord)
	{
		index[2].to_array(texcoord_index);
	}

	// Generate vb/ib
	uint32_t idx = 0;
	for (uint32_t i = 0; i < position_index.size(); i++)
	{
		MeshVertex v;

		uint16_t p_idx = position_index[i] * 3;
		v.position = Vector3(position_array[p_idx], position_array[p_idx + 1], position_array[p_idx + 2]);

		if (m_has_normal)
		{
			uint16_t n_idx = normal_index[i] * 3;
			v.normal = Vector3(normal_array[n_idx], normal_array[n_idx + 1], normal_array[n_idx + 2]);
		}

		if (m_has_texcoord)
		{
			uint16_t t_idx = texcoord_index[i] * 2;
			v.texcoord = Vector2(texcoord_array[t_idx], texcoord_array[t_idx + 1]);
		}


		uint32_t f_idx = 0;
		bool found = false;
		for (; f_idx < m_vertices.size(); f_idx++)
		{
			if (m_vertices[f_idx] == v)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			m_indices.push_back(f_idx);
		}
		else
		{
			m_vertices.push_back(v);
			m_indices.push_back(idx);
			idx++;
		}
	}

	m_mesh_header.version = MESH_VERSION;
	m_mesh_header.num_meshes = 1;
	m_mesh_header.num_joints = 0;
	//m_mesh_header.padding[0] = 0xCECECECE;

	default_allocator().deallocate(buf);
	fs.close(file);

	MeshData data;
	data.vertices.num_vertices = m_vertices.size();
	data.vertices.format = VertexFormat::P3_N3_T2;
	data.vertices.offset = sizeof(MeshHeader) + sizeof(MeshData);

	data.indices.num_indices = m_indices.size();
	data.indices.offset = sizeof(MeshHeader) + sizeof(MeshData) + m_vertices.size() * sizeof(MeshVertex);

	// Write header
	out_file->write((char*)&m_mesh_header, sizeof(MeshHeader));

	// Write mesh metadata
	out_file->write((char*)&data, sizeof(MeshData));

	// Write vertices
	out_file->write((char*) m_vertices.begin(), m_vertices.size() * sizeof(MeshVertex));

	// Write indices
	out_file->write((char*) m_indices.begin(), m_indices.size() * sizeof(uint16_t));

	// Cleanup
	m_vertices.clear();
	m_indices.clear();
}

} // namespace mesh_resource
} // namespace crown
