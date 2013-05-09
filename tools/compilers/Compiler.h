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
#include "Filesystem.h"

namespace crown
{

const size_t MAX_RESOURCE_NAME_LENGTH = 1024;
const size_t MAX_RESOURCE_TYPE_LENGTH = 64;
const size_t MAX_RESOURCE_PATH_LENGTH = 1024;

/// Contains the header data common to all
/// types of resources passing through the
/// standard Compiler mechanics.
struct CompiledHeader
{
	uint32_t	name;	// Name of the resource (murmur2_32 hash)
	uint32_t	type;	// Type of the resource (murmur2_32 hash)
	uint32_t	size;	// Size of the resource data _not_ including header (in bytes)
};

class FileStream;

/// Resource compiler interface.
/// Every specific resource compiler must inherith from this
/// interface and implement its methods accordingly.
class Compiler
{
public:

	/// Looks for the @resource int the @root_path and prepares it to
	/// compilation using @seed to generate hashes for the resource name.
	/// Implementation must declare the type of resource they are expecting
	/// to work on by setting @type_expected appropriately.
						Compiler(const char* root_path, const char* dest_path, const char* resource,
								 uint32_t type_expected, uint32_t seed);
	virtual				~Compiler();

	/// Actually compiles the resource.
	virtual bool		compile() = 0;

	virtual void		write() = 0;

	const char*			root_path() const;
	const char*			dest_path() const;
	const char*			resource_path() const;

	uint32_t			resource_name_hash() const;
	uint32_t			resource_type_hash() const;
	uint32_t			seed() const;

	const char*			resource_name() const;
	const char*			resource_type() const;

	FileStream*			source_file();
	FileStream*			destination_file();

protected:

	void				prepare_header(uint32_t size);
	void				write_header();

private:

	// These memebers are private to prevent
	// derived classes from manipulating them.

	// Generic informations
	char				m_root_path[MAX_RESOURCE_PATH_LENGTH];
	char				m_dest_path[MAX_RESOURCE_PATH_LENGTH];
	char				m_resource[MAX_RESOURCE_PATH_LENGTH];

	uint32_t			m_name_hash;
	uint32_t			m_type_hash;
	uint32_t			m_seed;

	char				m_name[MAX_RESOURCE_NAME_LENGTH];
	char				m_type[MAX_RESOURCE_TYPE_LENGTH];

	// Filesystems
	Filesystem			m_root_fs;
	Filesystem			m_dest_fs;

	FileStream*			m_src_file;
	FileStream*			m_dest_file;

	// Compilation stage
	CompiledHeader		m_compiled_header;

	bool				m_prepared;

	// Global compiler settings
	bool				m_verbose;
};

} // namespace crown

