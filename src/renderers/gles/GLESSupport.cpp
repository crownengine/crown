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

#include <GLES/gl.h>
#include "GLESSupport.h"
#include "Str.h"
#include "Types.h"

namespace Crown
{

GLESSupport::GLESSupport()
{
}

GLESSupport::~GLESSupport()
{
}

const Str& GLESSupport::GetVendor() const
{
	return mVendor;
}

const Str& GLESSupport::GetRenderer() const
{
	return mRenderer;
}

const Str& GLESSupport::GetVersion() const
{
	return mVersion;
}

bool GLESSupport::CheckExtension(Str extension) const
{
	if (mExtensionList.Find(extension) == -1)
	{
		return false;
	}

	return true;
}

void GLESSupport::BuildExtensionList()
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

GLESSupport* GetGLESSupport()
{
	return new GLESSupport();
}

} // namespace Crown

