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
#include "Types.h"
#include "String.h"
#include "Args.h"
#include <cstdlib>

// #include "renderers/gl/GLRenderer.h"
#include "renderers/gles/GLESRenderer.h"

namespace crown
{

const uint16_t Device::CROWN_MAJOR = 0;
const uint16_t Device::CROWN_MINOR = 1;
const uint16_t Device::CROWN_MICRO = 0;

//-----------------------------------------------------------------------------
Device::Device() :
	mPreferredWindowWidth(1000),
	mPreferredWindowHeight(625),
	mPreferredWindowFullscreen(false),
	mIsInit(false),
	mIsRunning(false),
	mRenderer(NULL)
{
	string::strcpy(mPreferredRootPath, string::EMPTY);
	string::strcpy(mPreferredUserPath, string::EMPTY);
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

	// Set the root path
	// GetFilesystem()->Init(mPreferredRootPath.c_str(), mPreferredUserPath.c_str());

	// Create the main window
	if (!os::create_render_window(0, 0, mPreferredWindowWidth, mPreferredWindowHeight, mPreferredWindowFullscreen))
	{
		Log::E("Unable to create the main window.");
		return false;
	}
	Log::D("Window created.");

	// Create the renderer
	if (!mRenderer)
	{
		// FIXME FIXME FIXME
		// #ifdef CROWN_BUILD_OPENGL
		// 	mRenderer = new GLRenderer();
		// #elif defined CROWN_BUILD_OPENGLES
			mRenderer = new GLESRenderer();
		// #endif
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
		delete mRenderer;
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

	get_input_manager()->event_loop();

		mRenderer->begin_frame();
		mRenderer->end_frame();

	os::swap_buffers();
}

//-----------------------------------------------------------------------------
bool Device::ParseCommandLine(int argc, char** argv)
{
	int32_t fullscreen = 0;

	ArgsOption options[] = 
	{
		"help",       AOA_NO_ARGUMENT,       NULL,        'i',
		"root-path",  AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"user-path",  AOA_REQUIRED_ARGUMENT, NULL,        'u',
		"width",      AOA_REQUIRED_ARGUMENT, NULL,        'w',
		"height",     AOA_REQUIRED_ARGUMENT, NULL,        'h',
		"fullscreen", AOA_NO_ARGUMENT,       &fullscreen,  1,
		NULL, 0, NULL, 0
	};

	Args args(argc, argv, "", options);

	while (1)
	{
		int32_t ret = args.next_option();

		switch (ret)
		{
			case -1:
			{
				return true;
			}
			case 0:
			{
				mPreferredWindowFullscreen = fullscreen;

				break;
			}
			// Help
			case 'i':
			{
				PrintHelpMessage();
				return false;
			}
			// Root path
			case 'r':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `-root-path`\n", argv[0]);
					return false;
				}

				string::strcpy(mPreferredRootPath, args.option_argument());

				break;
			}
			// User path
			case 'u':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `--user-path`\n", argv[0]);
					return false;
				}

				string::strcpy(mPreferredUserPath, args.option_argument());

				break;
			}
			// Window width
			case 'w':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing width value after `--width`\n", argv[0]);
					return false;
				}

				mPreferredWindowWidth = atoi(args.option_argument());
				break;
			}
			// Window height
			case 'h':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing height value after `--height`\n", argv[0]);
					return false;
				}

				mPreferredWindowHeight = atoi(args.option_argument());
				break;
			}
			default:
			{
				break;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void Device::PrintHelpMessage()
{
	os::printf("Usage: crown [options]\n");
	os::printf("Options:\n\n");

	os::printf("All of the following options take precedence over\n");
	os::printf("environment variables and configuration files.\n\n");

	os::printf("  --help                Show this help.\n");
	os::printf("  --root-path <path>    Use <path> as the filesystem root path.\n");
	os::printf("  --user-path <path>    Use <path> as the filesystem user path.\n");
	os::printf("  --width <width>       Set the <width> of the render window.\n");
	os::printf("  --height <width>      Set the <height> of the render window.\n");
	os::printf("  --fullscreen          Start in fullscreen.\n");
}

Device device;
Device* GetDevice()
{
	return &device;
}

} // namespace crown

