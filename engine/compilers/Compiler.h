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

#include "Types.h"

namespace crown
{

class Filesystem;
class File;

/// Resource compiler interface.
/// Every specific resource compiler must implement this interface.
class Compiler
{
public:

	virtual					~Compiler() {}

	/// Compiles the @name_in resource coming from @a root_path
	/// in a engine-ready format and puts it into @a dest_path with the @a name_out name.
	/// Returns true whether the copilation was successfull, false otherwise.
	bool					compile(const char* root_path, const char* dest_path, const char* name_in, const char* name_out);
	
	/// Clears all the temporary stuctures used to compile
	/// the resource.
	void					cleanup();

protected:

	/// Compiles the resource found at @a resource path. A Filesystem instance is
	/// passed along to be able to read resource data. The implementer must care
	/// of returning the total size in bytes of the compiled resource or 0 if an
	/// error occurs.
	virtual size_t			compile_impl(Filesystem& fs, const char* resource_path) = 0;

	/// Writes the compiled resource to @a out_file.
	virtual void			write_impl(File* out_file) = 0;
};

} // namespace crown
