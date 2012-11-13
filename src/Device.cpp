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

#include "Config.h"
#include "Device.h"
#include "Filesystem.h"
#include "InputManager.h"
#include "Log.h"
#include "OS.h"
#include "Renderer.h"
#include "Timer.h"
#include "Types.h"
#include <cstdlib>

namespace crown
{

const ushort Device::CROWN_MAJOR = 0;
const ushort Device::CROWN_MINOR = 1;
const ushort Device::CROWN_MICRO = 0;

//-----------------------------------------------------------------------------
Device::Device() :
	mPreferredWindowWidth(1000),
	mPreferredWindowHeight(625),
	mPreferredWindowFullscreen(false),
	mPreferredRootPath(Str::EMPTY),
	mPreferredUserPath(Str::EMPTY),
	mIsInit(false),
	mIsRunning(false),
	mRenderer(NULL)
{
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
bool Device::Init(int argc, char** argv)
{
	if (ParseCommandLine(argc, argv) == false)
	{
		return false;
	}

	// Initialize
	Log::D("Initializing Device...");

	if (IsInit())
	{
		Log::E("Device is already initialized.");
		return false;
	}

	// Sets the root path
	GetFilesystem()->Init(mPreferredRootPath.c_str(), mPreferredUserPath.c_str());

	// Creates the main window
	if (!os::create_render_window(0, 0, mPreferredWindowWidth, mPreferredWindowHeight, mPreferredWindowFullscreen))
	{
		Log::E("Unuble to create the main window.");
		return false;
	}
	Log::D("Window created.");

	// Creates the renderer
	if (!mRenderer)
	{
		mRenderer = Renderer::CreateRenderer();
	}
	Log::D("Renderer created.");

	os::init_input();

	mIsInit = true;

	StartRunning();

	Log::I("Crown Game Engine %d.%d.%d", CROWN_MAJOR, CROWN_MINOR, CROWN_MICRO);
	Log::I("Crown is up and running, enjoy!");

	return true;
}

//-----------------------------------------------------------------------------
void Device::Shutdown()
{
	if (!IsInit())
	{
		Log::E("Device is not initialized.");	
		return;
	}

	Log::I("Releasing Renderer...");

	if (mRenderer)
	{
		Renderer::DestroyRenderer(mRenderer);
	}

	Log::I("Releasing Render Window...");
	os::destroy_render_window();

	mIsInit = false;
}

//-----------------------------------------------------------------------------
bool Device::IsInit()
{
	return mIsInit;
}

//-----------------------------------------------------------------------------
Renderer* Device::GetRenderer()
{
	return mRenderer;
}

//-----------------------------------------------------------------------------
void Device::StartRunning()
{
	if (!IsInit())
	{
		return;
	}

	mIsRunning = true;
}

//-----------------------------------------------------------------------------
void Device::StopRunning()
{
	if (!IsInit())
	{
		return;
	}

	mIsRunning = false;
}

//-----------------------------------------------------------------------------
bool Device::IsRunning() const
{
	return mIsRunning;
}

//-----------------------------------------------------------------------------
void Device::Frame()
{
	os::event_loop();

	GetInputManager()->EventLoop();

		mRenderer->_BeginFrame();
		mRenderer->_EndFrame();

	os::swap_buffers();
}

//-----------------------------------------------------------------------------
bool Device::ParseCommandLine(int argc, char** argv)
{
	if (argc == 2 && Str::StrCmp(argv[1], "-help") == 0)
	{
		os::printf("Usage: %s [options]\n", argv[0]);
		os::printf("Options:\n\n");
		os::printf("All of the following options take precedence over\nenvironment variables and configuration files.\n\n");
		// Print options
		os::printf("  -help\t\t\t\t\tShow this help\n");
		os::printf("  -root-path <path>\t\t\tUse <path> as the filesystem root path\n");
		os::printf("  -user-path <path>\t\t\tUse <path> as the filesystem user path\n");
		os::printf("  -metrics <width> <height>\t\tSet the <width> and <height> of the render window\n");
		os::printf("  -fullscreen\t\t\t\tStart in fullscreen\n");

		return false;
	}

	// Parse command line arguments
	int i = 1;
	while (i < argc)
	{
		if (Str::StrCmp(argv[i], "-root-path") == 0)
		{
			if (argc < i + 2)
			{
				os::printf("%s: error: missing absolute path after `-root-path`\n", argv[0]);
				return false;
			}
			mPreferredRootPath = argv[i + 1];
			// Two arguments crunched
			i += 2;
			continue;
		}
		if (Str::StrCmp(argv[i], "-user-path") == 0)
		{
			if (argc < i + 2)
			{
				os::printf("%s: error: missing absolute path after `-user-path`\n", argv[0]);
				return false;
			}
			mPreferredUserPath = argv[i + 1];
			// Two arguments crunched
			i += 2;
			continue;
		}
		if (Str::StrCmp(argv[i], "-metrics") == 0)
		{
			if (argc < i + 3)
			{
				os::printf("%s: error: wrong number of arguments after `-metrics`\n", argv[0]);
				return false;
			}
			mPreferredWindowWidth = atoi(argv[i + 1]);
			mPreferredWindowHeight = atoi(argv[i + 2]);
			// Three arguments crunched
			i += 3;
			continue;
		}
		if (Str::StrCmp(argv[i], "-fullscreen") == 0)
		{
			mPreferredWindowFullscreen = true;
			// One argument crunched
			i++;
			continue;
		}

		// Next argument
		i++;
	}

	return true;
}

Device device;
Device* GetDevice()
{
	return &device;
}

} // namespace crown

