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

#include <GL/glew.h>
#include "GLSupport.h"
#include "Str.h"
#include "Types.h"

namespace Crown
{

GLSupport::GLSupport() :
	mVendor("unknown"),
	mRenderer("unknown"),
	mVersion("unknown")
{
}

GLSupport::~GLSupport()
{
}

const Str& GLSupport::GetVendor() const
{
	return mVendor;
}

const Str& GLSupport::GetRenderer() const
{
	return mRenderer;
}

const Str& GLSupport::GetVersion() const
{
	return mVersion;
}

bool GLSupport::CheckExtension(const Str& extension) const
{
	if (mExtensionList.Find(extension) == -1)
	{
		return false;
	}

	return true;
}

void GLSupport::BuildExtensionList()
{
	char* string = (char*)glGetString(GL_VENDOR);
	mVendor = string;
	string = (char*)glGetString(GL_RENDERER);
	mRenderer = string;
	string = (char*)glGetString(GL_VERSION);
	mVersion = string;
	string = (char*)glGetString(GL_EXTENSIONS);
	Str extensions = string;
	extensions.Split(' ', mExtensionList);
}

GLSupport glSupport;
GLSupport* GetGLSupport()
{
	return &glSupport;
}

} // namespace Crown

