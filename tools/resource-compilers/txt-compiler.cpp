#include <stdio.h>
#include <stdlib.h>

#include "Crown.h"

using namespace crown;

const char* root_path = NULL;
const char* resource_in = NULL;
const char* resource_out = NULL;

void print_help_message(const char* program_name);
void parse_command_line(int argc, char** argv);

/// UTF-8 compiler for "txt" resource types
int main(int argc, char** argv)
{
	parse_command_line(argc, argv);
	
	// FIXME: validate input

	Filesystem fs_root(root_path);
	
	if (!fs_root.exists(resource_in))
	{
		printf("%s: ERROR: %s does not exist. Aborting.\n", resource_in);
		return -1;
	}

	char resource_basename[256];
	char resource_extension[256];
	
	path::filename_without_extension(resource_in, resource_basename, 256);
	path::extension(resource_in, resource_extension, 256);
	
	uint32_t resource_basename_hash = hash::fnv1a_32(resource_basename, string::strlen(resource_basename));
	uint32_t resource_extension_hash = hash::fnv1a_32(resource_extension, string::strlen(resource_extension));

	FileStream* src_file = (FileStream*)fs_root.open(resource_in, SOM_READ);
	
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

void parse_command_line(int argc, char** argv)
{
	// Parse arguments
	ArgsOption options[] = 
	{
		"help",         AOA_NO_ARGUMENT,       NULL,        'h',
		"root-path",    AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"resource-in",  AOA_REQUIRED_ARGUMENT, NULL,        'i',
		"resource-out", AOA_REQUIRED_ARGUMENT, NULL,        'o',
		NULL, 0, NULL, 0
	};

	Args args(argc, argv, "", options);

	while (1)
	{
		int32_t ret = args.next_option();
		
		switch (ret)
		{
			case -1:
			{
				return;
			}
			// Help message
			case 'h':
			{
				print_help_message(argv[0]);
				exit(0);
			}
			// Root path
			case 'r':
			{
				if (args.option_argument() == NULL)
				{
					printf("%s: ERROR: missing path after `--root-path`\n", argv[0]);
					exit(-1);
				}
				
				root_path = args.option_argument();
				
				break;
			}
			// Resource in
			case 'i':
			{
				if (args.option_argument() == NULL)
				{
					printf("%s: ERROR: missing path after `--resource-in`\n", argv[0]);
					exit(-1);
				}
				
				resource_in = args.option_argument();
				
				break;
			}
			// Resource out
			case 'o':
			{
				if (args.option_argument() == NULL)
				{
					printf("%s: ERROR: missing path after `--resource-out`\n", argv[0]);
					exit(-1);
				}

				resource_out = args.option_argument();
				
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void print_help_message(const char* program_name)
{
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n\n");

	printf("  --help                  Show this help.\n");
	printf("  --root-path <path>      The _absolute_ <path> whether to look for the input resource.\n");
	printf("  --resource-in <path>    The _relative_ <path> of the input resource.\n");
	printf("  --resource-out <width>  The _relative_ <path> of the output resource.\n");
}

