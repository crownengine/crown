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

#include "Compiler.h"
#include "Hash.h"
#include "Path.h"
#include "FileStream.h"
#include <cstring>

namespace crown
{

//-----------------------------------------------------------------------------
Compiler::Compiler(const char* root_path, const char* dest_path, const char* resource, uint32_t seed) :
	m_name_hash(0),
	m_type_hash(0),
	m_seed(seed),

	m_root_fs(root_path),
	m_dest_fs(dest_path),
	m_src_file(NULL),
	m_dest_file(NULL),

	m_prepared(false),

	m_verbose(false)
{
	// Init structures
	memset(m_root_path, 0, MAX_RESOURCE_PATH_LENGTH);
	memset(m_dest_path, 0, MAX_RESOURCE_PATH_LENGTH);
	memset(m_resource, 0, MAX_RESOURCE_PATH_LENGTH);

	memset(&m_compiled_header, 0, sizeof(CompiledHeader));
	memset(m_name, 0, MAX_RESOURCE_NAME_LENGTH);
	memset(m_type, 0, MAX_RESOURCE_TYPE_LENGTH);

	string::strncpy(m_root_path, root_path, MAX_RESOURCE_PATH_LENGTH);
	string::strncpy(m_dest_path, dest_path, MAX_RESOURCE_PATH_LENGTH);
	string::strncpy(m_resource, resource, MAX_RESOURCE_PATH_LENGTH);

	// Extract resource name and type
	path::filename_without_extension(m_resource, m_name, MAX_RESOURCE_NAME_LENGTH);
	path::extension(m_resource, m_type, MAX_RESOURCE_TYPE_LENGTH);

	// Compute hashes
	m_name_hash = hash::murmur2_32(m_name, string::strlen(m_name), m_seed);
	m_type_hash = hash::murmur2_32(m_type, string::strlen(m_type), m_seed);

	char dest_name[17];
	memset(dest_name, 0, 17);

	snprintf(dest_name, 17, "%X%X", m_name_hash, m_type_hash);

	// Open streams
	m_src_file = (FileStream*)m_root_fs.open(m_resource, SOM_READ);

	if (!m_dest_fs.exists(dest_name))
	{
		m_dest_fs.create_file(dest_name);
	}

	m_dest_file = (FileStream*)m_dest_fs.open(dest_name, SOM_WRITE);
}

//-----------------------------------------------------------------------------
Compiler::~Compiler()
{
	if (m_src_file != NULL)
	{
		m_root_fs.close(m_src_file);
	}

	if (m_dest_file != NULL)
	{
		m_dest_fs.close(m_dest_file);
	}
}

//-----------------------------------------------------------------------------
const char* Compiler::root_path() const
{
	return m_root_path;
}

//-----------------------------------------------------------------------------
const char* Compiler::dest_path() const
{
	return m_dest_path;
}

//-----------------------------------------------------------------------------
const char* Compiler::resource_path() const
{
	return m_resource;
}

//-----------------------------------------------------------------------------
uint32_t Compiler::resource_name_hash() const
{
	return m_name_hash;
}

//-----------------------------------------------------------------------------
uint32_t Compiler::resource_type_hash() const
{
	return m_type_hash;
}

//-----------------------------------------------------------------------------
uint32_t Compiler::seed() const
{
	return m_seed;
}

//-----------------------------------------------------------------------------
const char* Compiler::resource_name() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
const char* Compiler::resource_type() const
{
	return m_type;
}

//-----------------------------------------------------------------------------
FileStream* Compiler::source_file()
{
	return m_src_file;
}

//-----------------------------------------------------------------------------
FileStream* Compiler::destination_file()
{
	return m_dest_file;
}

//-----------------------------------------------------------------------------
void Compiler::prepare_header(uint32_t size)
{
	m_compiled_header.name = m_name_hash;
	m_compiled_header.type = m_type_hash;
	m_compiled_header.size = size;
}

//-----------------------------------------------------------------------------
void Compiler::write_header()
{
	m_dest_file->write(&m_compiled_header, sizeof(CompiledHeader));
}

} // namespace crown

