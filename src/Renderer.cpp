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

#include "Renderer.h"
#include "Config.h"
#include "Exceptions.h"

#if defined(CROWN_USE_OPENGL)
	#include "GLRenderer.h"
#elif defined(CROWN_USE_OPENGLES)
	#include "GLESRenderer.h"
#endif

namespace Crown
{

Renderer* Renderer::CreateRenderer()
{
	Renderer* renderer = NULL;

	#if (defined(LINUX) || defined(WINDOWS)) && defined(CROWN_USE_OPENGL)
	renderer = new GLRenderer;
	#elif defined(CROWN_OS_ANDROID) || (defined(LINUX) && defined(CROWN_USE_OPENGLES))
	renderer = new GLESRenderer;
	#endif

	return renderer;
}

void Renderer::DestroyRenderer(Renderer* renderer)
{
	if (renderer == NULL)
	{
		throw NullPointerException("Renderer::DestroyRenderer: renderer == NULL");
	}

	delete renderer;
}

} // namespace Crown

