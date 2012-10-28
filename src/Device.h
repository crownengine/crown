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
#include "Timer.h"
#include "Str.h"
#include "EventLoop.h"

namespace Crown
{

class Renderer;
class RenderWindow;
class InputManager;
class GarbageBin;

/**
	Device.
*/
class Device
{

public:

							Device();
							~Device();

	RenderWindow*			GetMainWindow();
	InputManager*			GetInputManager();
	Renderer*				GetRenderer();
	GarbageBin*				GetGarbageBin();

	void					StartRunning();
	void					StopRunning();
	bool					IsRunning() const;
	bool					IsInit();
	void					Frame();

	bool					Init(int argc, char** argv);
	void					Shutdown();

private:

	bool					ParseCommandLine(int argc, char** argv);

	static const ushort		CROWN_MAJOR;
	static const ushort		CROWN_MINOR;
	static const ushort		CROWN_MICRO;

	int						mPreferredWindowWidth;
	int						mPreferredWindowHeight;
	bool					mPreferredWindowFullscreen;
	Str						mPreferredRootPath;
	Str						mPreferredUserPath;

	bool					mIsInit		: 1;
	bool					mIsRunning	: 1;

	RenderWindow*			mMainWindow;
	InputManager*			mInputManager;
	Renderer*				mRenderer;

	GarbageBin*				mGarbageBin;
	Timer					mTimer;

	EventLoop				_event_loop;

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

Device* GetDevice();

} // namespace Crown

