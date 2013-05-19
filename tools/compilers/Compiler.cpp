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

#include <cstring>
#include <cstdlib>
#include "Compiler.h"
#include "Hash.h"
#include "Path.h"
#include "FileStream.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
Compiler::Compiler(const char* root_path, const char* dest_path, uint32_t type_expected) :
	m_root_fs(root_path),
	m_dest_fs(dest_path),
	m_type_expected(type_expected)
{
	memset(m_resource_name, 0, MAX_RESOURCE_NAME_LENGTH);
}

//-----------------------------------------------------------------------------
Compiler::~Compiler()
{
}

//-----------------------------------------------------------------------------
size_t Compiler::compile(const char* resource, uint32_t name, uint32_t type)
{
	string::strncpy(m_resource_name, resource, MAX_RESOURCE_NAME_LENGTH);

	char resource_name[MAX_RESOURCE_NAME_LENGTH];
	char resource_type[MAX_RESOURCE_TYPE_LENGTH];

	path::filename_without_extension(resource, resource_name, MAX_RESOURCE_NAME_LENGTH);
	path::extension(resource, resource_type, MAX_RESOURCE_TYPE_LENGTH);

	char output_name[17];
	snprintf(output_name, 17, "%.8X%.8X", name, type);

	Log::i("%s => %s", resource, output_name);

	if (type != m_type_expected)
	{
		Log::e("'%s': resource type does not match expected type.", resource);
		return 0;
	}

	if (!m_root_fs.exists(resource))
	{
		Log::e("'%s': resource does not exist.");
		return 0;
	}

	// Read source file
	FileStream* input_file = m_root_fs.open(resource, SOM_READ);

	size_t header_size = read_header(input_file);
	size_t resource_size = read_resource(input_file);

	// Write compiled file
	FileStream* output_file;

	if (m_dest_fs.exists(output_name))
	{
		m_dest_fs.delete_file(output_name);
	}

	m_dest_fs.create_file(output_name);
	output_file = m_dest_fs.open(output_name, SOM_WRITE);

	write_header(output_file, name, type, resource_size);
	write_resource(output_file);

	m_root_fs.close(input_file);
	m_dest_fs.close(output_file);

	// Cleanup
	cleanup();
}

//-----------------------------------------------------------------------------
size_t Compiler::read_header(FileStream* in_file)
{
	return read_header_impl(in_file);
}

//-----------------------------------------------------------------------------
size_t Compiler::read_resource(FileStream* in_file)
{
	return read_resource_impl(in_file);
}

//-----------------------------------------------------------------------------
void Compiler::write_header(FileStream* out_file, uint32_t name, uint32_t type, uint32_t resource_size)
{
	CompiledHeader header;
	header.magic = COMPILED_HEADER_MAGIC_NUMBER;
	header.version = COMPILER_VERSION;
	header.name = name;
	header.type = type;
	header.size = resource_size;

	out_file->write(&header, sizeof(CompiledHeader));

	write_header_impl(out_file);
}

//-----------------------------------------------------------------------------
void Compiler::write_resource(FileStream* out_file)
{
	write_resource_impl(out_file);
}

//-----------------------------------------------------------------------------
void Compiler::cleanup()
{
	cleanup_impl();

	string::strncpy(m_resource_name, "", MAX_RESOURCE_NAME_LENGTH);
}

//-----------------------------------------------------------------------------
const char* Compiler::root_path() const
{
	return m_root_fs.root_path();
}

//-----------------------------------------------------------------------------
const char* Compiler::dest_path() const
{
	return m_dest_fs.root_path();
}

//-----------------------------------------------------------------------------
const char* Compiler::resource_name() const
{
	return m_resource_name;
}

} // namespace crown

