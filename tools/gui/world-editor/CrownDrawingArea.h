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

#include <gtkmm/drawingarea.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#include "Device.h"

namespace crown
{

// Custom widget to draw with OpenGL
class CrownDrawingArea : public Gtk::DrawingArea
{
public:

				CrownDrawingArea(Device* engine);
	virtual		~CrownDrawingArea();

	void		on_realize();
	bool		on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr);

	bool		on_idle();

protected:

	Display*	m_display;
	GLXContext	m_context;

	Device*		m_engine;

private:

	bool		make_current();
	void		swap_buffers();
};

} // namespace crown

