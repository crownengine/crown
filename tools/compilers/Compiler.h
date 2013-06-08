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
#include "Filesystem.h"

namespace crown
{

const size_t	MAX_RESOURCE_NAME_LENGTH			= 1024;
const size_t	MAX_RESOURCE_TYPE_LENGTH			= 64;
const size_t	MAX_RESOURCE_PATH_LENGTH			= 2048;

const uint32_t	COMPILED_HEADER_MAGIC_NUMBER		= 0xCE010101;
const uint32_t	COMPILER_VERSION					= 1;

/// Contains the header data common to all
/// types of resources passing through the
/// standard Compiler mechanics.
struct CompiledHeader
{
	uint32_t	magic;		// Magic number used to identify the file
	uint32_t	version;	// Version of the compiler used to compile the resource
	uint32_t	name;		// Name of the resource (murmur2_32 hash)
	uint32_t	type;		// Type of the resource (murmur2_32 hash)
	uint32_t	size;		// Size of the resource data _not_ including header (in bytes)
};

class DiskFile;

/// Resource compiler interface.
/// Every specific resource compiler must inherith from this
/// interface and implement its methods accordingly.
class Compiler
{
public:

							Compiler(const char* root_path, const char* dest_path, uint32_t type_expected);
	virtual					~Compiler();

	size_t					compile(const char* resource, uint32_t name, uint32_t type);

	size_t					read_header(DiskFile* in_file);
	size_t					read_resource(DiskFile* in_file);

	void					write_header(DiskFile* out_file, uint32_t name, uint32_t type, uint32_t resource_size);
	void					write_resource(DiskFile* out_file);

	void					cleanup();

	const char*				root_path() const;
	const char*				dest_path() const;
	const char*				resource_name() const;
	const char*				resource_path() const;

protected:

	virtual size_t			read_header_impl(DiskFile* in_file) = 0;
	virtual size_t			read_resource_impl(DiskFile* in_file) = 0;

	virtual void			write_header_impl(DiskFile* out_file) = 0;
	virtual void			write_resource_impl(DiskFile* out_file) = 0;

	virtual void			cleanup_impl() = 0;

private:

	// Filesystems
	Filesystem			m_root_fs;
	Filesystem			m_dest_fs;

	uint32_t			m_type_expected;

	char				m_resource_name[MAX_RESOURCE_NAME_LENGTH];
	char 				m_resource_path[MAX_RESOURCE_PATH_LENGTH];
};

} // namespace crown
