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

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Compiler.h"
#include "ResourceFormat.h"

namespace crown
{

//-----------------------------------------------------------------------------
size_t Compiler::compile(const char* root_path, const char* dest_path, const char* name_in, const char* name_out)
{
	std::string path_in = std::string(root_path) + "/" + std::string(name_in);
	std::string path_out = std::string(dest_path) + "/" + std::string(name_out);

	// The compilation fails when returned size is zero
	size_t resource_size = 0;
	if ((resource_size = compile_impl(path_in.c_str())) == 0)
	{
		std::cout << "Compilation failed." << std::endl;
		return 0;
	}

	// Setup resource header
	ResourceHeader resource_header;
	resource_header.magic = RESOURCE_MAGIC_NUMBER;
	resource_header.version = RESOURCE_VERSION;
	resource_header.size = resource_size;

	// Open destination file and write the header
	std::fstream out_file;
	out_file.open(path_out.c_str(), std::fstream::out | std::fstream::binary);

	if (!out_file.is_open())
	{
		std::cout << "Unable to write compiled file." << std::endl;
		return 0;
	}

	out_file.write((char*)&resource_header, sizeof(ResourceHeader));

	// Write resource-specific data
	write_impl(out_file);

	out_file.close();

	// Cleanup
	cleanup();
}

//-----------------------------------------------------------------------------
void Compiler::cleanup()
{
}

} // namespace crown

