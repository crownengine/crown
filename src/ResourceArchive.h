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

#include "Resource.h"

namespace crown
{

class Filesystem;
class FileStream;

const uint32_t ARCHIVE_VERSION	= 1;	// Version of the archive

struct ArchiveHeader
{
	uint32_t	version;			// The version number of the archive
	uint32_t	entries_count;		// Number of resource entries in the archive
	uint32_t	checksum;			// MD5 checksum of the archive
	uint8_t		padding[64];		// Padding for additional data
};

struct ArchiveEntry
{
	uint32_t	name;				// Name of the resource (fnv1a hash)
	uint32_t	type;				// Type of the resource (fnv1a hash)
	uint64_t	offset;				// First byte of the resource (as absolute offset)
	uint32_t	size;				// Size of the resource data (in bytes)
};

/// Structure of the archive
///
/// [ArchiveHeader]
/// [ArchiveEntry]
/// [ArchiveEntry]
/// ...
/// [ArchiveEntry]
/// [ResourceData]
/// [ResourceData]
/// ...
/// [ResourceData]
///
/// A valid archive must always have at least the archive header,
/// starting at byte 0 of the archive file.
///
/// Newer archive versions must be totally backward compatible
/// across minor engine releases, in order to be able to use
/// recent version of the engine with older game archives.

/// Source of resources
class ResourceArchive
{
public:

							ResourceArchive(Filesystem* filesystem);
							~ResourceArchive();

	void					open(const char* archive);
	void					close();

	FileStream*				find(ResourceId name);

private:

	Filesystem*				m_filesystem;
	
	FileStream*				m_archive_file;

	ArchiveEntry*			m_entries;
	uint32_t				m_entries_count;
};

} // namespace crown

