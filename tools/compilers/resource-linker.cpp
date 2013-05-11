#include <stdio.h>
#include "Filesystem.h"
#include "Stream.h"
#include "Path.h"
#include "String.h"
#include "Hash.h"
#include "Resource.h"
#include "ResourceArchive.h"
#include "FileStream.h"
#include <cstring>

using namespace crown;

/// Resource linker links together individual compiled resources into a
/// single binary blob ready to be loaded by Crown Engine.
/// Usage: resource-linker <root-path> [resource1, resource2, ..., resourceN]
/// The resources are put into the archive in the order they appear in the command line.
/// This allows to simplify the code and to decouple the linking process from the
/// placement optimization of the resources in the final archive.
int main(int argc, char** argv)
{
	//-------------------------------------------------------------------------
	// Preliminary checks
	//-------------------------------------------------------------------------
	
	if (argc != 3)
	{
		printf("Usage: %s /path/to/compiled resource.abc.o\n", argv[0]);
		return -1;
	}
	
	Filesystem fs_root(argv[1]);
	const char* resource_name = argv[2];
	
	// FIXME Check the existence of the resource!!!
	
	//-------------------------------------------------------------------------
	// Read the archive
	//-------------------------------------------------------------------------
	
	// Open the archive file for reading or create if does not exist
	if (!fs_root.exists("archive.bin"))
	{
		fs_root.create_file("archive.bin");
	}

	// Open the archive file for both reading and writing
	FileStream* archive = (FileStream*)fs_root.open("archive.bin", (StreamOpenMode)(SOM_READ | SOM_WRITE));
	
	// The archive header used throughout the code
	ArchiveHeader header;
	memset(&header, 0, sizeof(ArchiveHeader));
	
	// If the archive is empty
	if (archive->size() == 0)
	{
		// Initializes the archive header
		header.version = ARCHIVE_VERSION;
		header.entries_count = 0;
		header.checksum = 0; // FIXME Implement checksum
	}
	else if (archive->size() < sizeof(ArchiveHeader))
	{
		// If the archive is malformed (i.e. its size is lesser than sizeof(ArchiveHeader))
		printf("Fatal: the archive file is malformed. Aborting.\n");
		return -1;
	}
	else
	{
		// If the archive is well-formed, read the archive header
		archive->read(&header, sizeof(ArchiveHeader));
	}

	// In-Memory representation of the table of entries
	ArchiveEntry* entries = NULL;
	uint32_t entries_count = 0;
	
	// Read the table of entries if present
	if (header.entries_count > 0)
	{
		entries = new ArchiveEntry[header.entries_count];
		archive->read(entries, sizeof(ArchiveEntry) * header.entries_count);
		entries_count = header.entries_count;
	}

	//-------------------------------------------------------------------------
	// Read the resource
	//-------------------------------------------------------------------------

	// Open the resource
	FileStream* resource = (FileStream*)fs_root.open(resource_name, SOM_READ);
	
	// If the resource is malformed, abort
	if (resource->size() < sizeof(ArchiveEntry))
	{
		printf("Fatal: the resource file is malformed. Aborting.\n");
		return -1;
	}
	
	// The resource entry used throughout the code
	ArchiveEntry resource_entry;

	// Read the resource entry
	resource->read(&resource_entry, sizeof(ArchiveEntry));

	// In-Memory representation of the resource data
	uint8_t* resource_data = NULL;
	size_t resource_data_size = 0;
	
	// Read the resource data if present
	if (resource_entry.size > 0)
	{
		resource_data = new uint8_t[resource_entry.size];
		resource->read(resource_data, resource_entry.size);
		resource_data_size = resource_entry.size;
	}

	//-------------------------------------------------------------------------
	// Patch the resource offset and update the archive header
	//-------------------------------------------------------------------------
	
	// 1) Obtain the total resource data size
	size_t total_resource_data_size = 0;
	
	for (uint32_t i = 0; i < entries_count; i++)
	{		
		total_resource_data_size += entries[i].size;
	}
	
	// 2) Read the total resource data in memory (FIXME: ouch... need better strategy, potentially source of
	//    troubles in case of very large archive files!)
	uint8_t* total_resource_data = NULL;
	
	// Read the data only if it is actually there...
	if (total_resource_data_size > 0)
	{
		total_resource_data = new uint8_t[total_resource_data_size];
		
		// The file cursor is right at the start of data section :)
		archive->read(total_resource_data, total_resource_data_size);
	}
	
	// 3) Patch the previous resource entry offsets
	for (uint32_t i = 0; i < entries_count; i++)
	{
		// Shift everything "down" by the size of the new ArchiveEntry
		entries[i].offset += sizeof(ArchiveEntry);
	}

	// 4) Patch the new resource entry offset
	resource_entry.offset = + sizeof(ArchiveHeader) + sizeof(ArchiveEntry) * (entries_count + 1) + total_resource_data_size;
	
	// 5) Path the archive header
	header.entries_count += 1;
	
	// 6) Write the new header, the previous entries, the new entry, the previos resource data and the new resource data
	//    _IN_THAT_ORDER_
	
	archive->seek(0);

	// Write the new header
	archive->write(&header, sizeof(ArchiveHeader));
	
	// Write the previous entries only if they exist
	if (entries_count > 0)
	{
		archive->write(entries, sizeof(ArchiveEntry) * entries_count);
	}

	// Write the new resource entry
	archive->write(&resource_entry, sizeof(ArchiveEntry));
	
	// Write previous total resource data only if it exist
	if (total_resource_data_size > 0)
	{
		archive->write(total_resource_data, total_resource_data_size);
	}

	// Write the new resource data only if if exists (a new resource could have no data associated with it)
	if (resource_data_size > 0)
	{
		archive->write(resource_data, resource_data_size);
	}
	
	//-------------------------------------------------------------------------
	// Free data and close streams
	//-------------------------------------------------------------------------
	
	if (entries != NULL)
	{
		delete[] entries;
	}
	
	if (total_resource_data != NULL)
	{
		delete[] total_resource_data;
	}
	
	if (resource_data != NULL)
	{
		delete[] resource_data;
	}

	// Close the files, we are done
	fs_root.close(resource);
	fs_root.close(archive);
	
	return 0;
}

