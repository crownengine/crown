local ffi = require("ffi")

ffi.cdef
[[
	bool	mouse_button_pressed(uint32_t button);

	bool	mouse_button_released(uint32_t button);

	Vec2&	mouse_cursor_xy();

	void	mouse_set_cursor_xy(const Vec2& position);

	Vec2&	mouse_cursor_relative_xy();

	void	mouse_set_cursor_relative_xy(const Vec2& position);
]]

Mouse = {}

Mouse.button_pressed			= lib.mouse_button_pressed
Mouse.button_released			= lib.mouse_button_released
Mouse.cursor_xy					= lib.mouse_cursor_xy
Mouse.set_cursor_xy				= lib.mouse_set_cursor_xy
Mouse.cursor_relative_xy		= lib.mouse_cursor_relative_xy
Mouse.set_cursor_relative_xy	= lib.mouse_set_cursor_relative_xy