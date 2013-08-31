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

#include "Compiler.h"
#include "Filesystem.h"
#include "DiskFileSource.h"
#include "ResourceFormat.h"
#include "File.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Compiler::compile(const char* root_path, const char* dest_path, const char* name_in, const char* name_out)
{
	DiskFileSource root_disk(root_path);
	DiskFileSource dest_disk(dest_path);
	Filesystem root_fs(root_disk);
	Filesystem dest_fs(dest_disk);

	// The compilation fails when returned size is zero
	size_t resource_size = 0;
	if ((resource_size = compile_impl(root_fs, name_in)) == 0)
	{
		Log::e("Compilation failed");
		return false;
	}

	// Setup resource header
	ResourceHeader resource_header;
	resource_header.magic = RESOURCE_MAGIC_NUMBER;
	resource_header.version = RESOURCE_VERSION;
	resource_header.size = resource_size;

	// Open destination file and write the header
	File* out_file = dest_fs.open(name_out, FOM_WRITE);

	if (out_file)
	{
		// Write header
		out_file->write((char*)&resource_header, sizeof(ResourceHeader));

		// Write resource-specific data
		write_impl(out_file);

		dest_fs.close(out_file);

		cleanup();
		return true;
	}

	Log::e("Unable to write compiled file.");
	return false;
}

//-----------------------------------------------------------------------------
void Compiler::cleanup()
{
}

} // namespace crown

