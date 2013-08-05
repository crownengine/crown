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

#pragma once

#include <map>
#include <string>
#include <vector>

#include "Compiler.h"
#include "MeshFormat.h"
#include "tinyxml2.h"

using tinyxml2::XMLElement;
using std::vector;

namespace crown
{

//-----------------------------------------------------------------------------
struct DAEFloatArray
{
	std::string				id;		// UUID of the array
	std::vector<float>		array;
};

//-----------------------------------------------------------------------------
struct DAEParam
{
	std::string				name;
	std::string				type;
};

//-----------------------------------------------------------------------------
struct DAEAccessor
{
	std::string				source;
	uint32_t				count;
	uint32_t				stride;
	std::vector<DAEParam>	params;	
};

//-----------------------------------------------------------------------------
struct DAETechniqueCommon
{
	DAEAccessor				accessor;
};

//-----------------------------------------------------------------------------
struct DAESource
{
	std::string 			id;					// UUID of the source
	DAEFloatArray			float_array;		// Array of floats
	DAETechniqueCommon		technique_common;
};

//-----------------------------------------------------------------------------
struct DAEInput
{
	std::string				semantic;
	std::string				source;
	uint32_t				offset;
};

//-----------------------------------------------------------------------------
struct DAEVertices
{
	std::string				id;
	std::vector<DAEInput>	inputs;
};

//-----------------------------------------------------------------------------
struct DAEPolylist
{
	uint32_t				count;
	std::vector<DAEInput>	inputs;
	std::vector<uint32_t>	vcount;
	std::vector<uint32_t>	p;
};

//-----------------------------------------------------------------------------
struct DAEMesh
{
	std::vector<DAESource>	sources;
	DAEVertices				vertices;
	DAEPolylist				polylist;
};

//-----------------------------------------------------------------------------
struct DAEGeometry
{
	std::string				id;		// UUID of the geometry
	std::string				name;	// Name of the geometry
	DAEMesh					mesh;	// The mesh
};

//-----------------------------------------------------------------------------
struct DAEModel
{
	std::vector<DAEGeometry>	geometries;
};

class DAECompiler : public Compiler
{
public:

						DAECompiler();
						~DAECompiler();

	size_t				compile_impl(const char* resource_path);
	void				write_impl(std::fstream& out_file);

private:

	// The following functions return false if parsing fails, true otherwise
	static bool			parse_collada(const char* path, DAEModel& m);
	static bool			parse_geometry(XMLElement* geometry, DAEGeometry& g);
	static bool			parse_mesh(XMLElement* mesh, DAEMesh& m);
	static bool			parse_source(XMLElement* source, DAESource& s);
	static bool			parse_float_array(XMLElement* array, DAEFloatArray& a);
	static bool			parse_technique_common(XMLElement* technique, DAETechniqueCommon& t);
	static bool			parse_accessor(XMLElement* accessor, DAEAccessor& a);
	static bool			parse_param(XMLElement* param, DAEParam& p);
	static bool			parse_vertices(XMLElement* vertices, DAEVertices& v);
	static bool			parse_input(XMLElement* input, DAEInput& i);
	static bool			parse_polylist(XMLElement* polylist, DAEPolylist& p);

	bool				find_vertices(const DAEMesh& mesh, DAESource& source_out);
	bool				find_normals(const DAEMesh& mesh, DAESource& source_out);

	bool				extract_vertex_indices(const DAEMesh& mesh, vector<uint16_t>& indices_out);
	bool				extract_vertex_normals(const DAEMesh& mesh, vector<uint32_t>& indices_out);

private:

	MeshHeader			m_mesh_header;

	vector<float>		m_vertex_vertices;
	vector<uint16_t>	m_vertex_indices;
};

} // namespace crown
