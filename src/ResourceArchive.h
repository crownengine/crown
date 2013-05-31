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

class Filesystem;
class DiskFile;

class ResourceArchive
{
public:

	virtual					~ResourceArchive() {}

	/// Opens the resource file containing @name resource
	/// and returns a stream from which read the data from.
	/// @note
	/// The resource stream points exactly at the start
	/// of the useful resource data, so you do not have to
	/// care about skipping headers, metadatas and so on.
	virtual DiskFile*		open(ResourceId name) = 0;

	/// Closes the resource file.
	virtual void			close(DiskFile* resource) = 0;
};

} // namespace crown

