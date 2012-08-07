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

#include "InputManager.h"
#include "Config.h"
#include "Exceptions.h"
#include "Device.h"

#if defined(LINUX)
	#include "X11InputManager.h"
	#if defined (CROWN_USE_OPENGL)
		#include "GLXRenderWindow.h"
	#elif defined(CROWN_USE_OPENGLES)
		#include "EGLRenderWindow.h"
	#endif
#elif defined(WINDOWS) && defined(CROWN_USE_OPENGL)
	#include "WinInputManager.h"
	#include "WGLRenderWindow.h"
#elif defined(CROWN_OS_ANDROID)
	#include "AndroidInputManager.h"
	#include "EGLRenderWindow.h"
#endif

namespace Crown
{

InputManager* InputManager::CreateInputManager(RenderWindow* window)
{
	InputManager* manager = NULL;
	EventSource source;

	if (window == NULL)
	{
		throw NullPointerException("InputManager::CreateInputManager: window == NULL");
	}

	#if defined(LINUX)
		manager = new X11InputManager;
		#if defined(CROWN_USE_OPENGL)
			source.WindowHandle = static_cast<GLXRenderWindow*>(window)->GetXWindow();
		#elif defined(CROWN_USE_OPENGLES)
			source.WindowHandle = static_cast<EGLRenderWindow*>(window)->GetXWindow();
		#endif
	#elif defined(WINDOWS) && defined(CROWN_USE_OPENGL)
		manager = new WinInputManager;
		source.WindowHandle = (unsigned long)(static_cast<WGLRenderWindow*>(window)->GetWindowHandle());
	#elif defined(CROWN_OS_ANDROID)
		manager = new AndroidInputManager;
		source.AndroidApp = GetDevice()->_GetAndroidApp();
	#else
		#error "No operating system selected."
	#endif

	if (manager)
	{
		manager->Init(source);
	}

	return manager;
}

void InputManager::DestroyInputManager(InputManager* manager)
{
	if (manager == NULL)
	{
		throw NullPointerException("InputManager::DestroyInputManager: manager == NULL");
	}

	delete manager;
}

} // namespace Crown

