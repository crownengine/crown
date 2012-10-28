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

#include "Types.h"
#include "Config.h"
#include "Device.h"
#include "Filesystem.h"
#include "GarbageBin.h"
#include "InputManager.h"
#include "Log.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "GarbageBin.h"
#include "RenderWindow.h"
#include "Config.h"
#include "Filesystem.h"
#include "Timer.h"
#include "OS.h"

namespace Crown
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
	mMainWindow(NULL),
	mInputManager(NULL),
	mRenderer(NULL),
	mGarbageBin(NULL)
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
	Log::D("Device::Init() called.");

	if (IsInit())
	{
		Log::E("Device is already initialized.");
		return false;
	}

	// Sets the root path
	GetFilesystem()->Init(mPreferredRootPath.c_str(), mPreferredUserPath.c_str());

	if (!(mMainWindow = RenderWindow::CreateWindow(0, 0, mPreferredWindowWidth, mPreferredWindowHeight, 32, mPreferredWindowFullscreen)))
	{
		Log::E("Unuble to create the main window.");
		return false;
	}

	mGarbageBin = new GarbageBin();

	Log::D("Window created.");

	if (!mRenderer)
	{
		mRenderer = Renderer::CreateRenderer();
	}
	Log::D("Renderer created.");

	mInputManager = InputManager::CreateInputManager(mMainWindow);

	Mouse* mouse = NULL;
	Keyboard* keyboard = NULL;
	Touch* touch = NULL;

	if (mInputManager)
	{
		keyboard = mInputManager->GetKeyboard();
		mouse = mInputManager->GetMouse();
		touch = mInputManager->GetTouch();
	}

	if (mouse)
	{
		mouse->SetListener(mInputManager->GetEventDispatcher());
	}
	if (keyboard)
	{
		keyboard->SetListener(mInputManager->GetEventDispatcher());
	}
	if (touch)
	{
		touch->SetListener(mInputManager->GetEventDispatcher());
	}
	Log::D("Input created.");

	mIsInit = true;

	StartRunning();

	Log::I("Crown Game Engine %d.%d.%d", CROWN_MAJOR, CROWN_MINOR, CROWN_MICRO);
	Log::I("Crown is up and running, enjoy!");

	return true;
}

//-----------------------------------------------------------------------------
InputManager* Device::GetInputManager()
{
	return mInputManager;
}

//-----------------------------------------------------------------------------
void Device::Shutdown()
{
	if (!IsInit())
	{
		Log::E("Device is not initialized.");	
		return;
	}

	Log::I("Releasing GarbageBin...");

	if (mGarbageBin)
	{
		delete mGarbageBin;
	}

	Log::I("Releasing Renderer...");

	if (mRenderer)
	{
		Renderer::DestroyRenderer(mRenderer);
	}

	Log::I("Releasing InputManager...");
	if (mInputManager)
	{
		InputManager::DestroyInputManager(mInputManager);
	}

	Log::I("Releasing Main Window...");

	if (mMainWindow)
	{
		mMainWindow->_SetMain(false);
		RenderWindow::DestroyWindow(mMainWindow);
	}

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
RenderWindow* Device::GetMainWindow()
{
	return mMainWindow;
}

//-----------------------------------------------------------------------------
GarbageBin* Device::GetGarbageBin()
{
	return mGarbageBin;
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
	// TODO Replace this mess
	if (mInputManager)
	{
		if (mInputManager->IsMouseAvailable())
		{
			if (mInputManager->IsMouseAvailable())
			{
				mInputManager->GetMouse()->EventLoop();
			}
			if (mInputManager->IsKeyboardAvailable())
			{
				mInputManager->GetKeyboard()->EventLoop();
			}
			if (mInputManager->IsTouchAvailable())
			{
				mInputManager->GetTouch()->EventLoop();
			}
		}
	}

	mMainWindow->EventLoop();

	mRenderer->_BeginFrame();
	mRenderer->_EndFrame();

	mMainWindow->Update();

	mGarbageBin->Empty();
}

//-----------------------------------------------------------------------------
bool Device::ParseCommandLine(int argc, char** argv)
{
	if (argc == 2 && Str::StrCmp(argv[1], "-help") == 0)
	{
		OS::Printf("Usage: %s [options]\n", argv[0]);
		OS::Printf("Options:\n\n");
		OS::Printf("All of the following options take precedence over\nenvironment variables and configuration files.\n\n");
		// Print options
		OS::Printf("  -help\t\t\t\t\tShow this help\n");
		OS::Printf("  -root-path <path>\t\t\tUse <path> as the filesystem root path\n");
		OS::Printf("  -user-path <path>\t\t\tUse <path> as the filesystem user path\n");
		OS::Printf("  -metrics <width> <height>\t\tSet the <width> and <height> of the render window\n");
		OS::Printf("  -fullscreen\t\t\t\tStart in fullscreen\n");

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
				OS::Printf("%s: error: missing absolute path after `-root-path`\n", argv[0]);
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
				OS::Printf("%s: error: missing absolute path after `-user-path`\n", argv[0]);
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
				OS::Printf("%s: error: wrong number of arguments after `-metrics`\n", argv[0]);
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

} // namespace Crown

