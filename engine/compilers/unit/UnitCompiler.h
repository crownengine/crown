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

#include "Compiler.h"
#include "Resource.h"
#include "UnitResource.h"
#include "List.h"
#include "Types.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "JSONParser.h"

namespace crown
{

struct GraphNode
{
	StringId32 name;
	StringId32 parent;
	Vector3 position;
	Quaternion rotation;
};

struct GraphNodeDepth
{
	StringId32 name;
	uint32_t index;
	uint32_t depth;

	bool operator()(const GraphNodeDepth& a, const GraphNodeDepth& b)
	{
		return a.depth < b.depth;
	}
};

class CE_EXPORT UnitCompiler : public Compiler
{
public:

	UnitCompiler();

	size_t compile_impl(Filesystem& fs, const char* resource_path);
	void write_impl(File* out_file);

	void parse_node(JSONElement e);
	void parse_camera(JSONElement e);
	void parse_renderable(JSONElement e);
	void parse_actor(JSONElement e);

	uint32_t compute_link_depth(GraphNode& node);
	uint32_t find_node_index(StringId32 name);
	int32_t find_node_parent_index(StringId32 name);

private:

	List<GraphNode>			m_nodes;
	List<GraphNodeDepth>	m_node_depths;

	List<UnitCamera>		m_cameras;
	List<UnitRenderable>	m_renderables;
	List<UnitActor>			m_actors;
};

} // namespace crown
