#include "Device.h"
#include "ScriptSystem.h"
#include "Mouse.h"
#include "OS.h"

namespace crown
{

extern "C"
{
	bool	mouse_button_pressed(uint32_t button);

	bool	mouse_button_released(uint32_t button);

	Vec2&	mouse_cursor_xy();

	void	mouse_set_cursor_xy(const Vec2& position);

	Vec2&	mouse_cursor_relative_xy();

	void	mouse_set_cursor_relative_xy(const Vec2& position);
}

bool mouse_button_pressed(uint32_t button)
{	
	MouseButton b;

	switch(button)
	{
		case MB_LEFT:		
		{
			b = MB_LEFT;
		}
		case MB_MIDDLE:
		{
			b = MB_MIDDLE;
		}
		case MB_RIGHT:
		{
			b = MB_RIGHT;
		}
		default:
		{
			return false;
		}
	}

	return device()->mouse()->button_pressed(b);
}

bool mouse_button_released(uint32_t button)
{
	MouseButton b;

	switch(button)
	{
		case MB_LEFT:		
		{
			b = MB_LEFT;
		}
		case MB_MIDDLE:
		{
			b = MB_MIDDLE;
		}
		case MB_RIGHT:
		{
			b = MB_RIGHT;
		}
		default:
		{
			return false;
		}
	}


	return device()->mouse()->button_released(b);
}

Vec2& mouse_cursor_xy()
{

	Point2 tmp = device()->mouse()->cursor_xy();

	return device()->script_system()->next_vec2(tmp.x, tmp.y);
}

void mouse_set_cursor_xy(const Vec2& position)
{
	Point2 tmp;
	tmp.x = (uint32_t)position.x;
	tmp.y = (uint32_t)position.y;

	device()->mouse()->set_cursor_xy(tmp);
}

Vec2& mouse_cursor_relative_xy()
{
	Vec2 tmp = device()->mouse()->cursor_relative_xy();

	return device()->script_system()->next_vec2(tmp.x, tmp.y);
}

void mouse_set_cursor_relative_xy(const Vec2& position)
{
	device()->mouse()->set_cursor_relative_xy(position);
}

}