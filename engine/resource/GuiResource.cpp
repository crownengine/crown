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

#include "Types.h"
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
void parse_triangle(JSONElement triangle, List<float>& points, List<float>& colors)
{
	JSONElement point	= triangle.key("points");
	JSONElement color 	= triangle.key("color");

	point.array_value(points);
	color.array_value(colors);
}

//-----------------------------------------------------------------------------
void parse_image(JSONElement image, StringId64& material, List<float>& positions, List<float>& sizes, List<float>& colors)
{
	JSONElement mat			= image.key("material");
	JSONElement position 	= image.key("position");
	JSONElement size 		= image.key("size");
	JSONElement color 		= image.key("color");

	DynamicString material_name;
	mat.string_value(material_name);
	material_name += "material";

	material = hash::murmur2_64(material_name.c_str(), string::strlen(material_name.c_str()), 0);
	position.array_value(positions);
	size.array_value(sizes);
	color.array_value(colors);
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	// Out buffer
	List<GuiRectData> 		m_gui_rects(default_allocator());
	List<GuiTriangleData> 	m_gui_triangles(default_allocator());
	List<GuiImageData> 		m_gui_images(default_allocator());

	JSONParser json(buf);
	JSONElement root = json.root();

	List<float>			m_gui_position(default_allocator());
	List<float>			m_gui_size(default_allocator());

	root.key("position").array_value(m_gui_position);
	root.key("size").array_value(m_gui_size);

	// Parse & compile all rects
	List<float> 		m_rect_positions(default_allocator());
	List<float> 		m_rect_sizes(default_allocator());
	List<float> 		m_rect_colors(default_allocator());

	JSONElement rects = root.key("rects");
	uint32_t num_rects = rects.size();

	for (uint32_t i = 0; i < num_rects; i++)
	{
		parse_rect(rects[i], m_rect_positions, m_rect_sizes, m_rect_colors);

		GuiRectData rect;
		rect.position[0] = m_rect_positions[0];
		rect.position[1] = m_rect_positions[1];
		rect.size[0] = m_rect_sizes[0];
		rect.size[1] = m_rect_sizes[1];
		rect.color[0] = m_rect_colors[0];
		rect.color[1] = m_rect_colors[1];
		rect.color[2] = m_rect_colors[2];
		rect.color[3] = m_rect_colors[3];

		m_gui_rects.push_back(rect);

		m_rect_positions.clear();
		m_rect_sizes.clear();
		m_rect_colors.clear();
	}

	// Parse & compile all triangles
	List<float> m_triangle_points(default_allocator());
	List<float> m_triangle_colors(default_allocator());

	JSONElement triangles = root.key("triangles");
	uint32_t num_triangles = triangles.size();

	for (uint32_t i = 0; i < num_triangles; i++)
	{
		parse_triangle(triangles[i], m_triangle_points, m_triangle_colors);

		GuiTriangleData triangle;
		triangle.points[0] = m_triangle_points[0];
		triangle.points[1] = m_triangle_points[1];
		triangle.points[2] = m_triangle_points[2];
		triangle.points[3] = m_triangle_points[3];
		triangle.points[4] = m_triangle_points[4];
		triangle.points[5] = m_triangle_points[5];
		triangle.color[0] = m_triangle_colors[0];
		triangle.color[1] = m_triangle_colors[1];
		triangle.color[2] = m_triangle_colors[2];
		triangle.color[3] = m_triangle_colors[3];

		m_gui_triangles.push_back(triangle);

		m_triangle_points.clear();
		m_triangle_colors.clear();
	}

	// Parse & compile all images
	StringId64			m_image_material = 0;
	List<float> 		m_image_positions(default_allocator());
	List<float> 		m_image_sizes(default_allocator());
	List<float> 		m_image_colors(default_allocator());

	JSONElement images = root.key("images");
	uint32_t num_images = images.size();

	for (uint32_t i = 0; i < num_images; i++)
	{
		parse_image(images[i], m_image_material, m_image_positions, m_image_sizes, m_image_colors);

		GuiImageData image;
		image.material.id 	= m_image_material;
		image.position[0] 	= m_image_positions[0];
		image.position[1] 	= m_image_positions[1];
		image.size[0] 		= m_image_sizes[0];
		image.size[1] 		= m_image_sizes[1];
		image.color[0] 		= m_image_colors[0];
		image.color[1] 		= m_image_colors[1];
		image.color[2] 		= m_image_colors[2];
		image.color[3] 		= m_image_colors[3];

		m_gui_images.push_back(image);

		m_image_positions.clear();
		m_image_sizes.clear();
		m_image_colors.clear();
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	// Fill resource header
	GuiHeader h;
	h.position[0] = m_gui_position[0];
	h.position[1] = m_gui_position[1];
	h.size[0] = m_gui_size[0];
	h.size[1] = m_gui_size[1];
	h.num_rects = num_rects;
	h.num_triangles = num_triangles;
	h.num_images = num_images;
	h.rects_offset 	= sizeof(GuiHeader);
	h.triangles_offset = h.rects_offset + sizeof(GuiRectData) * m_gui_rects.size();
	h.images_offset = h.triangles_offset + sizeof(GuiTriangleData) * m_gui_triangles.size();

	// Write compiled resource
	out_file->write((char*) &h, sizeof(GuiHeader));
	out_file->write((char*) m_gui_rects.begin(), sizeof(GuiRectData) * h.num_rects);
	out_file->write((char*) m_gui_triangles.begin(), sizeof(GuiTriangleData) * h.num_triangles);
	out_file->write((char*) m_gui_images.begin(), sizeof(GuiImageData) * h.num_images);
}

} // namespace gui_resource
} // namespace crown