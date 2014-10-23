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

#include "disk_filesystem.h"
#include "container_types.h"
#include "crown.h"

namespace crown
{

class BundleCompiler
{
public:

	BundleCompiler(const char* source_dir, const char* bundle_dir);

	bool compile(const char* type, const char* name, Platform::Enum platform);

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile_all(Platform::Enum platform);

	void scan(const char* cur_dir, Vector<DynamicString>& files);

private:

	DiskFilesystem _source_fs;
	DiskFilesystem _bundle_fs;
};

namespace bundle_compiler
{
	bool main(bool do_compile, bool do_continue, Platform::Enum platform);
} // namespace bundle_compiler

namespace bundle_compiler_globals
{
	/// Creates the global resource compiler.
	void init(const char* source_dir, const char* bundle_dir);

	/// Destroys the global resource compiler.
	void shutdown();

	/// Returns the global resource compiler.
	/// Returns NULL if the compiler is not available on the
	/// running platform.
	BundleCompiler* compiler();
} // namespace bundle_compiler_globals
} // namespace crown
