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


#include <cstring>
#include <inttypes.h>

#include "Allocator.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "GuiResource.h"
#include "StringUtils.h"
#include "Log.h"


namespace crown
{
namespace gui_resource
{

//-----------------------------------------------------------------------------
void parse_rect(JSONElement rect, List<float>& positions, List<float>& sizes, List<float>& colors)
{
	JSONElement position 	= rect.key("position");
	JSONElement size 		= rect.key("size");
	JSONElement color 		= rect.key("color");

	position.array_value(positions);
	size.array_value(sizes);
	color.array_value(colors);
}

//-----------------------------------------------------------------------------
void parse_triangle(JSONElement rect, List<float>& points, List<float>& colors)
{
	JSONElement point	= rect.key("points");
	JSONElement color 	= rect.key("color");

	point.array_value(points);
	color.array_value(colors);
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	List<float>			m_gui_position(default_allocator());
	List<float>			m_gui_size(default_allocator());

	root.key("position").array_value(m_gui_position);
	root.key("size").array_value(m_gui_size);

	List<float> 		m_rect_positions(default_allocator());
	List<float> 		m_rect_sizes(default_allocator());
	List<float> 		m_rect_colors(default_allocator());

	// Parse all rects
	JSONElement rects = root.key("rects");
	uint32_t num_rects = rects.size();

	for (uint32_t i = 0; i < num_rects; i++)
	{
		parse_rect(rects[i], m_rect_positions, m_rect_sizes, m_rect_colors);
	}

	// Compile all rects
	List<float>	m_rect_vertices(default_allocator());
	List<uint16_t> m_rect_indices(default_allocator());

	uint32_t rprx = 0;
	uint32_t rclx = 0;
	uint32_t ridx = 0;
	for (uint32_t i = 0; i < num_rects; i++)
	{
		// first vertex
		m_rect_vertices.push_back(m_rect_positions[rprx]);
		m_rect_vertices.push_back(m_rect_positions[rprx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx]);
		m_rect_vertices.push_back(m_rect_colors[rclx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx+2]);
		m_rect_vertices.push_back(m_rect_colors[rclx+3]);

		// second vertex
		m_rect_vertices.push_back(m_rect_positions[rprx] + m_rect_sizes[rprx]);
		m_rect_vertices.push_back(m_rect_positions[rprx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx]);
		m_rect_vertices.push_back(m_rect_colors[rclx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx+2]);
		m_rect_vertices.push_back(m_rect_colors[rclx+3]);

		// third vertex
		m_rect_vertices.push_back(m_rect_positions[rprx] + m_rect_sizes[rprx]);
		m_rect_vertices.push_back(m_rect_positions[rprx+1] - m_rect_sizes[rprx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx]);
		m_rect_vertices.push_back(m_rect_colors[rclx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx+2]);
		m_rect_vertices.push_back(m_rect_colors[rclx+3]);

		// fourth vertex
		m_rect_vertices.push_back(m_rect_positions[rprx]);
		m_rect_vertices.push_back(m_rect_positions[rprx+1] - m_rect_sizes[rprx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx]);
		m_rect_vertices.push_back(m_rect_colors[rclx+1]);
		m_rect_vertices.push_back(m_rect_colors[rclx+2]);
		m_rect_vertices.push_back(m_rect_colors[rclx+3]);

		// indices
		m_rect_indices.push_back(ridx); m_rect_indices.push_back(ridx+1);
		m_rect_indices.push_back(ridx+1); m_rect_indices.push_back(ridx+2);
		m_rect_indices.push_back(ridx+2); m_rect_indices.push_back(ridx+3);
		m_rect_indices.push_back(ridx+3); m_rect_indices.push_back(ridx);

		rprx += 2;
		rclx += 4;
		ridx += 4;
	}

	// Parse all triangles
	List<float> m_triangle_points(default_allocator());
	List<float> m_triangle_colors(default_allocator());

	JSONElement triangles = root.key("triangles");
	uint32_t num_triangles = triangles.size();
	for (uint32_t i = 0; i < num_triangles; i++)
	{
		parse_triangle(triangles[i], m_triangle_points, m_triangle_colors);
	}

	// Compile all triangles
	List<float>	m_triangle_vertices(default_allocator());
	List<uint16_t> m_triangle_indices(default_allocator());

	uint32_t tpnx = 0;
	uint32_t tclx = 0;
	uint32_t tidx = 0;
	for (uint32_t i = 0; i < num_triangles; i++)
	{
		// first vertex
		m_triangle_vertices.push_back(m_triangle_points[tpnx]);
		m_triangle_vertices.push_back(m_triangle_points[tpnx+1]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+1]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+2]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+3]);

		// second vertex
		m_triangle_vertices.push_back(m_triangle_points[tpnx+2]);
		m_triangle_vertices.push_back(m_triangle_points[tpnx+3]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+1]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+2]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+3]);

		// third vertex
		m_triangle_vertices.push_back(m_triangle_points[tpnx+4]);
		m_triangle_vertices.push_back(m_triangle_points[tpnx+5]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+1]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+2]);
		m_triangle_vertices.push_back(m_triangle_colors[tclx+3]);

		m_triangle_indices.push_back(tidx); m_triangle_indices.push_back(tidx+1);
		m_triangle_indices.push_back(tidx+1); m_triangle_indices.push_back(tidx+2);
		m_triangle_indices.push_back(tidx+2); m_triangle_indices.push_back(tidx);

		tpnx += 6;
		tclx += 4;
		tidx += 3;
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	// Write compiled resource to out_file
	GuiHeader h;
	h.position[0] = m_gui_position[0];
	h.position[1] = m_gui_position[1];
	h.size[0] = m_gui_size[0];
	h.size[1] = m_gui_size[1];

	h.num_rects = num_rects;
	h.num_triangles = num_triangles;

	h.rect_vertices_off = sizeof(GuiHeader);
	h.rect_indices_off = h.rect_vertices_off + sizeof(float) * m_rect_vertices.size();
	h.triangle_vertices_off = h.rect_indices_off + sizeof(uint16_t) * m_rect_indices.size();
	h.triangle_indices_off = h.triangle_vertices_off + sizeof(float) * m_triangle_vertices.size();

	out_file->write((char*) &h, sizeof(GuiHeader));
	out_file->write((char*) m_rect_vertices.begin(), sizeof(float) * m_rect_vertices.size());
	out_file->write((char*) m_rect_indices.begin(), sizeof(uint16_t) * m_rect_indices.size());
	out_file->write((char*) m_triangle_vertices.begin(), sizeof(float) * m_triangle_vertices.size());
	out_file->write((char*) m_triangle_indices.begin(), sizeof(uint16_t) * m_triangle_indices.size());
}

} // namespace gui_resource
} // namespace crown