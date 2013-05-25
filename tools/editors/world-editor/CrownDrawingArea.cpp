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

#include "CrownDrawingArea.h"
#include <iostream>
#include <gdk/gdkx.h>
#include "Assert.h"
#include <cstdio>
#include <GL/gl.h>
#include <gtkmm/window.h>
#include <glibmm.h>

namespace crown
{

//-----------------------------------------------------------------------------
CrownDrawingArea::CrownDrawingArea(Device* engine) :
	Gtk::DrawingArea(),
	m_engine(engine)
{
	Gtk::Widget::set_double_buffered(false);

	Display* dpy = gdk_x11_get_default_xdisplay();
	int xscreen = DefaultScreen(dpy);
	GdkScreen* screen = gdk_screen_get_default();

	// Color index buffer not supported - deprecated
	int32_t fbAttribs[] =
	{
		GLX_DOUBLEBUFFER,		static_cast<int32_t>(True),
		GLX_RED_SIZE,			8,
		GLX_GREEN_SIZE,			8,
		GLX_BLUE_SIZE,			8,
		GLX_ALPHA_SIZE,			8,
		GLX_DEPTH_SIZE,			24,
		GLX_STENCIL_SIZE,		0,
		GLX_ACCUM_RED_SIZE,		0,
		GLX_ACCUM_GREEN_SIZE,	0,
		GLX_ACCUM_BLUE_SIZE,	0,
		GLX_ACCUM_ALPHA_SIZE,	0,
		GLX_RENDER_TYPE,		static_cast<int32_t>(GLX_RGBA_BIT),
		GLX_DRAWABLE_TYPE,		static_cast<int32_t>(GLX_WINDOW_BIT),
		GLX_X_RENDERABLE,		static_cast<int32_t>(True),
		GLX_CONFIG_CAVEAT,		static_cast<int32_t>(GLX_DONT_CARE),
		GLX_TRANSPARENT_TYPE,	static_cast<int32_t>(GLX_NONE),
		static_cast<int32_t>(None)
	};

	int32_t fbCount;
	GLXFBConfig* fbConfig = glXChooseFBConfig(dpy, xscreen, fbAttribs, &fbCount);

	if (!fbConfig)
	{
		printf("Unable to find a matching FrameBuffer configuration.");
		return;
	}

	XVisualInfo* visualInfo = glXGetVisualFromFBConfig(dpy, fbConfig[0]);

	if (!visualInfo)
	{
		printf("Unable to find a matching Visual for the FrameBuffer configuration.");
		XFree(fbConfig);
		return;
	}
 
	XVisualInfo* xvisual = visualInfo;
	GdkVisual* visual = gdk_x11_screen_lookup_visual(screen, xvisual->visualid);

	
	gtk_widget_set_visual(GTK_WIDGET(this->gobj()), visual);

	m_context = glXCreateContext(dpy, xvisual, NULL, True);

	XFree(visualInfo);
	XFree(fbConfig);
	XFlush(dpy);

	m_display = dpy;
}

//-----------------------------------------------------------------------------
CrownDrawingArea::~CrownDrawingArea()
{
	if (m_context)
	{
		if (m_context == glXGetCurrentContext())
		{
			glXMakeCurrent(m_display, None, NULL);
		}

		glXDestroyContext(m_display, m_context);
	}
}

//-----------------------------------------------------------------------------
bool CrownDrawingArea::make_current()
{
	Glib::RefPtr<Gdk::Window> gdk_win = get_window();

	return (glXMakeContextCurrent(m_display,
								  GDK_WINDOW_XID(gdk_win->gobj()),
								  GDK_WINDOW_XID(gdk_win->gobj()),
								  m_context) == True);
}

//-----------------------------------------------------------------------------
void CrownDrawingArea::swap_buffers()
{
	Glib::RefPtr<Gdk::Window> gdk_win = get_window();

	glXSwapBuffers(m_display, GDK_WINDOW_XID(gdk_win->gobj()));
}

//-----------------------------------------------------------------------------
void CrownDrawingArea::on_realize()
{
	Gtk::DrawingArea::on_realize();

	make_current();

	char* argv[] = {"crown", "--dev" };

	m_engine->init(2, argv);

	Glib::signal_idle().connect(sigc::mem_fun(*this, &CrownDrawingArea::on_idle));
}

//-----------------------------------------------------------------------------
bool CrownDrawingArea::on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr)
{
	(void)cr;
	return true;
}

//-----------------------------------------------------------------------------
bool CrownDrawingArea::on_idle()
{
	make_current();

	m_engine->frame();

	swap_buffers();

	return true;
}

} // namespace crown

