#include "config.h"

#if CROWN_PLATFORM_OSX

#include "os_window_osx.h"
#include "ceassert.h"
#include "string_utils.h"
#include "log.h"

namespace crown
{

NSWindow* m_ns_window = NULL;

void oswindow_set_window(NSWindow* win)
{
	m_ns_window = win;
}

OsWindow::OsWindow()
	: m_x(0)
	, m_y(0)
	, m_width(0)
	, m_height(0)
	, m_resizable(true)
{
}

OsWindow::~OsWindow()
{
}

void OsWindow::show()
{

}

void OsWindow::hide()
{

}

void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

void OsWindow::resize(uint32_t width, uint32_t height)
{

}

void OsWindow::move(uint32_t x, uint32_t y)
{

}

void OsWindow::minimize()
{
	
}

void OsWindow::restore()
{
	
}

bool OsWindow::is_resizable() const
{
	return m_resizable;
}

void OsWindow::set_resizable(bool resizable)
{
	m_resizable = resizable;
}

char* OsWindow::title()
{
	static char title[1024];

	return title;
}

void OsWindow::set_title(const char* title)
{
	NSString *t = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
	[m_ns_window setTitle:t];
}

} // namespace crown

#endif // CROWN_PLATFORM_LINUX
