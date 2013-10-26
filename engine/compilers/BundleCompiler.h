/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "MeshCompiler.h"
#include "TextureCompiler.h"
#include "LuaCompiler.h"
#include "SoundCompiler.h"
#include "PackageCompiler.h"
#include "DynamicString.h"
#include "Vector.h"
#include "DiskFilesystem.h"
#include "UnitCompiler.h"

namespace crown
{

class CE_EXPORT BundleCompiler
{
public:

	BundleCompiler();

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// If @a resource is not NULL, only that particular resource is compiled.
	/// Returns true on success, false otherwise.
	bool compile(const char* bundle_dir, const char* source_dir, const char* resource = NULL);

private:

	static void scan(const char* source_dir, const char* cur_dir, Vector<DynamicString>& files);

private:

	MeshCompiler	m_mesh;
	TextureCompiler	m_texture;
	LuaCompiler 	m_lua;
	SoundCompiler	m_sound;
	PackageCompiler m_package;
	UnitCompiler	m_unit;
};

} // namespace crown
