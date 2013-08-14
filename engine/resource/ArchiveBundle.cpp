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

#include "ArchiveBundle.h"
#include "Filesystem.h"
#include "Resource.h"
#include "DiskFile.h"
#include "Log.h"
#include "Memory.h"

namespace crown
{

//-----------------------------------------------------------------------------
ArchiveBundle::ArchiveBundle(Filesystem& fs) :
	m_filesystem(fs),
	m_archive_file(NULL),
	m_entries_count(0),
	m_entries(NULL)
{
	// FIXME Default archive name
	m_archive_file = (DiskFile*)m_filesystem.open("disk", "archive.bin", FOM_READ);
	
	ArchiveHeader header;
	
	// Read the header of the archive
	m_archive_file->read(&header, sizeof(ArchiveHeader));
	
	Log::d("Version: %d", header.version);
	Log::d("Entries: %d", header.entries_count);
	Log::d("Checksum: %d", header.checksum);
	
	// No need to initialize memory
	m_entries = (ArchiveEntry*)m_allocator.allocate(header.entries_count * sizeof(ArchiveEntry));

	m_entries_count = header.entries_count;

	// Read the entries
	m_archive_file->read(m_entries, m_entries_count * sizeof(ArchiveEntry));
}

//-----------------------------------------------------------------------------
ArchiveBundle::~ArchiveBundle()
{
	if (m_archive_file != NULL)
	{
		m_filesystem.close(m_archive_file);
	}
	
	if (m_entries != NULL)
	{
		m_allocator.deallocate(m_entries);
	}
	
	m_entries = NULL;
	m_entries_count = 0;
}

//-----------------------------------------------------------------------------
DiskFile* ArchiveBundle::open(ResourceId name)
{
	// Search the resource in the archive
	for (uint32_t i = 0; i < m_entries_count; i++)
	{		
		if (m_entries[i].name == name.name && m_entries[i].type == name.type)
		{
			// If found, seek to the first byte of the resource data
			m_archive_file->seek(m_entries[i].offset);

			return (DiskFile*)m_archive_file;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void ArchiveBundle::close(DiskFile* resource)
{
	// Does nothing, the stream is automatically closed at exit.
	(void)resource;
}

} // namespace crown
