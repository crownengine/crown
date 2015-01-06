/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "win_headers.h"
#include "types.h"
#include "macros.h"

namespace crown
{

void oswindow_set_window(HWND handle_win);

struct OsWindow
{
public:

	OsWindow();
	~OsWindow();

	void show();
	void hide();

	void resize(uint32_t width, uint32_t height);
	void move(uint32_t x, uint32_t y);

	void minimize();
	void restore();

	bool is_resizable() const;
	void set_resizable(bool resizable);

	char* title();
	void set_title(const char* title);

public:

	char m_title[32];
	bool m_resizable;
};

} // namespace crown
