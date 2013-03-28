#include <stdio.h>
#include "Filesystem.h"
#include "Stream.h"
#include "Path.h"
#include "String.h"
#include "Hash.h"
#include "Resource.h"
#include "ResourceArchive.h"
#include "FileStream.h"

using namespace crown;

/// UTF-8 compiler for "txt" resource types
int main(int argc, char** argv)
{
	if (argc != 4)
	{
		printf("Usage: %s /path/to/resources resource_in.txt resource_out.txt\n", argv[0]);
		return -1;
	}

	Filesystem fs_root(argv[1]);

	const char* resource = argv[2];
	const char* resource_out = argv[3];
	
	if (!fs_root.exists(resource))
	{
		printf("Fatal: resource %s does not exists. Aborting.\n", resource);
		return -1;
	}
	
	char resource_basename[256];
	char resource_extension[256];
	
	path::filename_without_extension(resource, resource_basename, 256);
	path::extension(resource, resource_extension, 256);
	
	printf("Resource basename  : %s\n", resource_basename);
	printf("Resource extension : %s\n", resource_extension);
	
	uint32_t resource_basename_hash = hash::fnv1a_32(resource_basename, string::strlen(resource_basename));
	uint32_t resource_extension_hash = hash::fnv1a_32(resource_extension, string::strlen(resource_extension));
	
	printf("Resource basename  (hash) : %X\n", resource_basename_hash);
	printf("Resource extension (hash) : %X\n", resource_extension_hash);

	FileStream* src_file = (FileStream*)fs_root.open(resource, SOM_READ);
	
	size_t src_file_size = src_file->size();
	
	ArchiveEntry archive_entry;
	archive_entry.name = resource_basename_hash;
	archive_entry.type = resource_extension_hash;
	archive_entry.offset = sizeof (ArchiveEntry);
	archive_entry.size = src_file_size + sizeof(uint32_t);
	
	void* buffer = new uint8_t[src_file_size];
	
	src_file->read(buffer, src_file_size);
	
	fs_root.close(src_file);
	
	
	FileStream* dest_file = (FileStream*)fs_root.open(resource_out, SOM_WRITE);

	dest_file->write(&archive_entry, sizeof(ArchiveEntry));
	dest_file->write(&src_file_size, sizeof(uint32_t));
	dest_file->write(buffer, src_file_size);

	fs_root.close(dest_file);

	return 0;
}

