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
#include "List.h"
#include "Str.h"

namespace crown
{

class RenderWindow;

class GLESSupport
{

	typedef List<Str> StrList;

public:

	//! Constructor
	GLESSupport();

	//! Destructor
	virtual ~GLESSupport();

	//! Returns the name of the vendor
	const Str& GetVendor() const;

	//! Returns the name of the renderer
	const Str& GetRenderer() const;

	//! Returns the OpenGL's version number
	const Str& GetVersion() const;

	//! Returns true if the extension is supported
	bool CheckExtension(Str extension) const;

	//! Builds the extension list
	void BuildExtensionList();

protected:

	Str mVendor;
	Str mRenderer;
	Str mVersion;

	StrList mExtensionList;
};

GLESSupport* GetGLESSupport();

} // namespace crown

