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

#include "Allocator.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "PhysicsResource.h"
#include "StringUtils.h"

namespace crown
{
namespace physics_resource
{

//-----------------------------------------------------------------------------
void parse_controller(JSONElement e, PhysicsController& controller)
{
	JSONElement name = e.key("name");
	JSONElement height = e.key("height");
	JSONElement radius = e.key("radius");
	JSONElement slope_limit = e.key("slope_limit");
	JSONElement step_offset = e.key("step_offset");
	JSONElement contact_offset = e.key("contact_offset");

	controller.name = hash::murmur2_32(name.string_value(), name.size(), 0);
	controller.height = height.float_value();
	controller.radius = radius.float_value();
	controller.slope_limit = slope_limit.float_value();
	controller.step_offset = step_offset.float_value();
	controller.contact_offset = contact_offset.float_value();
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	bool m_has_controller = false;
	PhysicsController m_controller;

	// Read controller
	JSONElement controller = root.key_or_nil("controller");
	if (controller.is_nil())
	{
		m_has_controller = false;
	}
	else
	{
		parse_controller(controller, m_controller);
		m_has_controller = true;
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	PhysicsHeader h;
	h.version = 1;
	h.num_controllers = m_has_controller ? 1 : 0;

	uint32_t offt = sizeof(PhysicsHeader);
	h.controller_offset = offt;

	out_file->write((char*) &h, sizeof(PhysicsHeader));

	if (m_has_controller)
	{
		out_file->write((char*) &m_controller, sizeof(PhysicsController));
	}
}

} // namespace physics_resource
} // namespace crown
