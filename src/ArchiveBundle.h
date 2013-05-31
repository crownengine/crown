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
#include "Bundle.h"
#include "MallocAllocator.h"

namespace crown
{

class Filesystem;
class DiskFile;

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
class ArchiveBundle : public Bundle
{
public:

					ArchiveBundle(Filesystem& fs);
					~ArchiveBundle();

	/// @copydoc Bundle::open()
	DiskFile*		open(ResourceId name);

	/// @copydoc Bundle::close()
	void			close(DiskFile* resource);

private:

	MallocAllocator	m_allocator;

	Filesystem&		m_filesystem;

	DiskFile*		m_archive_file;

	uint32_t		m_entries_count;
	ArchiveEntry*	m_entries;
};

} // namespace crown
