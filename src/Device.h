/*
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

#include "Types.h"
#include "Config.h"

namespace crown
{

class Filesystem;
class Renderer;
class InputManager;
class Game;

/// The Engine.
class Device
{
public:

							Device();
							~Device();

	bool					init(int argc, char** argv);
	void					shutdown();

	bool					is_running() const;
	bool					is_init() const;

	void					start();
	void					stop();

	void					frame();

	Filesystem*				filesystem();
	InputManager*			input_manager();
	Renderer*				renderer();

private:

	bool					parse_command_line(int argc, char** argv);
	void					print_help_message();

private:

	// Preferred settings from command line
	int32_t					m_preferred_window_width;
	int32_t					m_preferred_window_height;
	bool					m_preferred_window_fullscreen;

	char					m_preferred_root_path[512];
	char					m_preferred_user_path[512];

	bool					m_is_init		: 1;
	bool					m_is_running	: 1;

	// Subsystems
	Filesystem*				m_filesystem;
	InputManager*			m_input_manager;
	Renderer*				m_renderer;

	// The game currently running
	Game*					m_game;
	void*					m_game_library;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

Device* device();

} // namespace crown

