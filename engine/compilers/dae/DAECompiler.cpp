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

#include <iostream>
#include <sstream>
#include "DAECompiler.h"

using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XML_NO_ERROR;
using tinyxml2::XML_NO_ATTRIBUTE;
using std::vector;
using std::cout;
using std::endl;

namespace crown
{

//-----------------------------------------------------------------------------
DAECompiler::DAECompiler()
{

}

//-----------------------------------------------------------------------------
DAECompiler::~DAECompiler()
{

}

//-----------------------------------------------------------------------------
size_t DAECompiler::compile_impl(const char* resource_path)
{
	DAEModel model;

	if (!parse_collada(resource_path, model))
	{
		return 0;
	}

	// Check that polylist is composed of triangles
	const vector<uint32_t>& vcount = model.geometries[0].mesh.polylist.vcount;
	for (uint32_t i = 0; i < vcount.size(); i++)
	{
		if (vcount[i] != 3)
		{
			cout << "Bad polylist: only triangle primitives allowed." << endl;
			return false;
		}
	}

	// Find vertices
	DAESource vertex_source;
	if (!find_vertices(model.geometries[0].mesh, vertex_source))
	{
		return false;
	}

	const vector<float>& vertex_vertices = vertex_source.float_array.array;
	m_vertex_vertices = vertex_vertices;

	vector<uint16_t> vertex_indices;
	if (!extract_vertex_indices(model.geometries[0].mesh, vertex_indices))
	{
		return false;
	}

	m_vertex_indices = vertex_indices;

	// cout << "Vertices: " << vertex_vertices.size() << endl;
	// cout << "Indices: " << vertex_indices.size() << endl;

	m_mesh_header.version = MESH_VERSION;
	m_mesh_header.mesh_count = 1;
	m_mesh_header.joint_count = 0;
	//m_mesh_header.padding[0] = 0xCECECECE;

	return sizeof(MeshHeader) +
			sizeof(uint32_t) + vertex_vertices.size() * sizeof(float) +
			sizeof(uint32_t) + vertex_indices.size() * sizeof(uint16_t);
}

//-----------------------------------------------------------------------------
void DAECompiler::write_impl(std::fstream& out_file)
{
	out_file.write((char*) &m_mesh_header, sizeof(MeshHeader));

	uint32_t vertex_count = m_vertex_vertices.size();
	out_file.write((char*)&vertex_count, sizeof(uint32_t));
	out_file.write((char*) m_vertex_vertices.data(), m_vertex_vertices.size() * sizeof(float));

	uint32_t triangle_count = m_vertex_indices.size();
	out_file.write((char*)&triangle_count, sizeof(uint32_t));
	out_file.write((char*) m_vertex_indices.data(), m_vertex_indices.size() * sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_collada(const char* path, DAEModel& m)
{
	XMLDocument doc;

	if (doc.LoadFile(path) != XML_NO_ERROR)
	{
		cout << "Unable to open '" << path << "'." << endl;
		return false;
	}

	// Read root
	XMLElement* root_node = doc.FirstChildElement("COLLADA");
	if (root_node == NULL)
	{
		cout << "Bad document: missing COLLADA root element." << endl;
		return false;
	}

	// Read geometries
	XMLElement* library_geometries = root_node->FirstChildElement("library_geometries");
	if (library_geometries == NULL)
	{
		cout << "Bad document: missing geometries library." << endl;
		return false;
	}

	XMLElement* geometry = library_geometries->FirstChildElement("geometry");
	if (geometry == NULL)
	{
		cout << "Bad document: no geometries found." << endl;
		return false;
	}

	for (; geometry != NULL; geometry = geometry->NextSiblingElement("geometry"))
	{
		DAEGeometry g;
		if (parse_geometry(geometry, g))
		{
			m.geometries.push_back(g);
		}
		else
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_geometry(XMLElement* geometry, DAEGeometry& g)
{
	// Read geometry id
	const char* geom_id = geometry->Attribute("id");
	if (geom_id == NULL)
	{
		cout << "Bad geometry: missing 'id' attribute." << endl;
		return false;
	}

	g.id = geom_id;

	// Read geometry name
	const char* geom_name = geometry->Attribute("name");
	if (geom_name == NULL)
	{
		cout << "Bad geometry: missing 'name' attribute." << endl;
		return false;
	}

	g.name = geom_name;

	// Read geometry mesh
	XMLElement* mesh = geometry->FirstChildElement("mesh");
	if (mesh == NULL)
	{
		cout << "Bad geometry: no meshes found." << endl;
		return false;
	}

	// Actually, there is more stuff to parse
	return parse_mesh(mesh, g.mesh);
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_mesh(XMLElement* mesh, DAEMesh& m)
{
	/// Read sources
	XMLElement* source = mesh->FirstChildElement("source");
	if (source == NULL)
	{
		cout << "Bad mesh: no sources found." << endl;
		return false;
	}

	for (; source != NULL; source = source->NextSiblingElement("source"))
	{
		DAESource s;
		if (parse_source(source, s))
		{
			m.sources.push_back(s);
		}
		else
		{
			return false;
		}
	}

	// Read vertices
	XMLElement* vertices = mesh->FirstChildElement("vertices");
	if (vertices == NULL)
	{
		cout << "Bad mesh: no vertices found." << endl;
		return false;
	}

	if (!parse_vertices(vertices, m.vertices))
	{
		return false;
	}

	// Read polylist
	XMLElement* polylist = mesh->FirstChildElement("polylist");
	if (polylist == NULL)
	{
		cout << "Bad mesh: no polylist found." << endl;
		return false;
	}

	return parse_polylist(polylist, m.polylist);
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_source(XMLElement* source, DAESource& s)
{
	// Read source id
	const char* source_id = source->Attribute("id");
	if (source_id == NULL)
	{
		cout << "Bad source: missing 'id' attribute." << endl;
		return false;
	}

	s.id = source_id;

	// Read float array
	XMLElement* float_array = source->FirstChildElement("float_array");
	if (float_array == NULL)
	{
		cout << "Bad source: no 'float_array' found." << endl;
		return false;
	}

	if (!parse_float_array(float_array, s.float_array))
	{
		return false;
	}

	// Read technique
	XMLElement* technique_common = source->FirstChildElement("technique_common");
	if (technique_common == NULL)
	{
		cout << "Bad source: no 'technique_common' found." << endl;
		return false;
	}

	if (!parse_technique_common(technique_common, s.technique_common))
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_float_array(XMLElement* array, DAEFloatArray& a)
{
	// Read float array id
	const char* float_array_id = array->Attribute("id");
	if (float_array_id == NULL)
	{
		cout << "Bad float array: missing 'id' attribute." << endl;
		return false;
	}

	a.id = float_array_id;

	// Read number of floats
	uint32_t count = 0;
	if (array->QueryUnsignedAttribute("count", &count) != XML_NO_ERROR)
	{
		cout << "Bad float array: missing 'count' attribute." << endl;
		return false;
	}

	std::istringstream floats(array->GetText());
	for (uint32_t i = 0; i < count; i++)
	{
		float out = 0.0f;
		floats >> out;
		a.array.push_back(out);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_technique_common(XMLElement* technique, DAETechniqueCommon& t)
{
	// Read accessor
	XMLElement* accessor = technique->FirstChildElement("accessor");
	if (accessor == NULL)
	{
		cout << "Bad technique: no accessors found." << endl;
		return false;
	}

	return parse_accessor(accessor, t.accessor);
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_accessor(XMLElement* accessor, DAEAccessor& a)
{
	// Read accessor source
	const char* accessor_source = accessor->Attribute("source");
	if (accessor_source == NULL)
	{
		cout << "Bad accessor: missing 'source' attribute." << endl;
		return false;
	}

	// First char is '#'
	a.source = &accessor_source[1];

	// Read accessor count
	if (accessor->QueryUnsignedAttribute("count", &a.count) != XML_NO_ERROR)
	{
		cout << "Bad accessor: missing 'count' attribute." << endl;
		return false;
	}

	// Read accessor stride
	if (accessor->QueryUnsignedAttribute("stride", &a.stride) != XML_NO_ERROR)
	{
		cout << "Bad accessor: missing 'stride' attribute." << endl;
		return false;
	}

	// Read params
	XMLElement* param = accessor->FirstChildElement("param");
	if (param == NULL)
	{
		cout << "Bad accessor: no params found." << endl;
		return false;
	}

	for (; param != NULL; param = param->NextSiblingElement("param"))
	{
		DAEParam p;
		if (parse_param(param, p))
		{
			a.params.push_back(p);
		}
		else
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_param(XMLElement* param, DAEParam& p)
{
	// Read param name
	const char* param_name = param->Attribute("name");
	if (param_name == NULL)
	{
		cout << "Bad param: missing 'name' attribute." << endl;
		return false;
	}

	p.name = param_name;

	// Read param type
	const char* param_type = param->Attribute("type");
	if (param_type == NULL)
	{
		cout << "Bad param: missing 'type' attribute." << endl;
		return false;
	}

	p.type = param_type;

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_vertices(XMLElement* vertices, DAEVertices& v)
{
	// Read vertices id
	const char* vertices_id = vertices->Attribute("id");
	if (vertices_id == NULL)
	{
		cout << "Bad vertices: missing 'id' attribute." << endl;
		return false;
	}

	v.id = vertices_id;

	// Read inputs
	XMLElement* input = vertices->FirstChildElement("input");
	if (input == NULL)
	{
		cout << "Bad vertices: no inputs found." << endl;
		return false;
	}

	for (; input != NULL; input = input->NextSiblingElement("input"))
	{
		DAEInput i;
		if (parse_input(input, i))
		{
			v.inputs.push_back(i);
		}
		else
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_input(XMLElement* input, DAEInput& i)
{
	// Read input semantic
	const char* input_semantic = input->Attribute("semantic");
	if (input_semantic == NULL)
	{
		cout << "Bad input: missing 'semantic' attribute." << endl;
		return false;
	}

	i.semantic = input_semantic;

	// Read input source
	const char* input_source = input->Attribute("source");
	if (input_source == NULL)
	{
		cout << "Bad input: missing 'source' attribute." << endl;
		return false;
	}

	// First char is '#'
	i.source = &input_source[1];

	if (input->QueryUnsignedAttribute("offset", &i.offset) == XML_NO_ATTRIBUTE)
	{
		i.offset = 0;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::parse_polylist(XMLElement* polylist, DAEPolylist& p)
{
	// Read polylist count
	if (polylist->QueryUnsignedAttribute("count", &p.count) != XML_NO_ERROR)
	{
		cout << "Bad polylist: missing 'count' attribute." << endl;
		return false;
	}

	// Read inputs
	XMLElement* input = polylist->FirstChildElement("input");
	if (input == NULL)
	{
		cout << "Bad polylist: no inputs found." << endl;
		return false;
	}

	for (; input != NULL; input = input->NextSiblingElement("input"))
	{
		DAEInput i;
		if (parse_input(input, i))
		{
			p.inputs.push_back(i);
		}
		else
		{
			return false;
		}
	}

	// Read vcount
	XMLElement* vcount = polylist->FirstChildElement("vcount");
	if (vcount == NULL)
	{
		cout << "Bad polylist: no vcount found." << endl;
		return false;
	}

	std::istringstream vcount_text(vcount->GetText());
	uint32_t vcount_item = 0;
	while (vcount_text >> vcount_item)
	{
		p.vcount.push_back(vcount_item);
	}

	// Read p
	XMLElement* p_element = polylist->FirstChildElement("p");
	if (p_element == NULL)
	{
		cout << "Bad polylist: no p found." << endl;
		return false;
	}

	std::istringstream p_text(p_element->GetText());
	uint32_t p_item = 0;
	while (p_text >> p_item)
	{
		p.p.push_back(p_item);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool DAECompiler::find_vertices(const DAEMesh& mesh, DAESource& source_out)
{
	const vector<DAESource>& sources = mesh.sources;
	const vector<DAEInput>& inputs = mesh.vertices.inputs;

	for (uint32_t input_i = 0; input_i < inputs.size(); input_i++)
	{
		const DAEInput& input = inputs[input_i];

		for (uint32_t source_i = 0; source_i < sources.size(); source_i++)
		{
			const DAESource& source = sources[source_i];

			if (input.semantic == "POSITION" && input.source == source.id)
			{
				source_out = source;
				return true;
			}
		}
	}

	cout << "Failed to find 'POSITION' source." << endl;
	return false;
}

//-----------------------------------------------------------------------------
bool DAECompiler::find_normals(const DAEMesh& mesh, DAESource& source_out)
{
	const vector<DAESource>& sources = mesh.sources;
	const vector<DAEInput>& inputs = mesh.polylist.inputs;

	for (uint32_t input_i = 0; input_i < inputs.size(); input_i++)
	{
		const DAEInput& input = inputs[input_i];

		for (uint32_t source_i = 0; source_i < sources.size(); source_i++)
		{
			const DAESource& source = sources[source_i];

			cout << input.semantic << " " << input.source << " " << source.id << endl;
			if (input.semantic == "NORMAL" && input.source == source.id)
			{
				source_out = source;
				return true;
			}
		}
	}

	cout << "Failed to find 'NORMAL' source." << endl;
	return false;
}

//-----------------------------------------------------------------------------
bool DAECompiler::extract_vertex_indices(const DAEMesh& mesh, vector<uint16_t>& indices_out)
{
	// Find vertices
	DAESource vertex_source;
	if (!find_vertices(mesh, vertex_source))
	{
		return false;
	}

	// Read vertices
	const vector<uint32_t>& primitives = mesh.polylist.p;
	const vector<float>& vertices = vertex_source.float_array.array;

	// FIXME FIXME FIXME
	uint32_t offset = 0;
	uint32_t attribs = mesh.polylist.inputs.size();

	uint32_t prims = 0;
	while(prims < primitives.size())
	{
		indices_out.push_back((uint16_t) primitives[prims]);
		prims += attribs;
	}

	return true;
}

} // namespace crown
