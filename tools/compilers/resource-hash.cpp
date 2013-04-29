#include <stdio.h>
#include <stdlib.h>

#include "Crown.h"

using namespace crown;

const char* resource_in = NULL;
uint32_t hash_seed = 0;

void print_help_message(const char* program_name);
void parse_command_line(int argc, char** argv);

/// Generates 64bit hash for the specified resource based on a seed
int main(int argc, char** argv)
{
	parse_command_line(argc, argv);

	if (resource_in == NULL)
	{
		printf("%s: ERROR: you have to specify the resource name with `--resource-in`\n", argv[0]);
		exit(-1);
	}

	char resource_basename[256];
	char resource_extension[256];
	
	path::filename_without_extension(resource_in, resource_basename, 256);
	path::extension(resource_in, resource_extension, 256);
	
	uint32_t resource_basename_hash = hash::murmur2_32(resource_basename, string::strlen(resource_basename), hash_seed);
	uint32_t resource_extension_hash = hash::murmur2_32(resource_extension, string::strlen(resource_extension), hash_seed);

	char out_filename[512];
	out_filename[0] = '\0';

	snprintf(resource_basename, 256, "%X", resource_basename_hash);
	snprintf(resource_extension, 256, "%X", resource_extension_hash);
	
	string::strncat(out_filename, resource_basename, 512);
	string::strncat(out_filename, resource_extension, 512);

	printf("%s\n", out_filename);

	return 0;
}

//-----------------------------------------------------------------------------
void parse_command_line(int argc, char** argv)
{
	// Parse arguments
	ArgsOption options[] = 
	{
		"help",         AOA_NO_ARGUMENT,       NULL,        'h',
		"resource-in",  AOA_REQUIRED_ARGUMENT, NULL,        'i',
		"seed",         AOA_REQUIRED_ARGUMENT, NULL,        's',
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
			case 's':
			{
				if (args.option_argument() == NULL)
				{
					printf("%s: ERROR: missing seed value after `--seed`\n", argv[0]);
					exit(-1);
				}

				hash_seed = atoi(args.option_argument());

				break;
			}
			default:
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void print_help_message(const char* program_name)
{
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n\n");

	printf("  --help                  Show this help.\n");
	printf("  --resource-in <name>    The <name> of the input resource.\n");
	printf("  --seed <value>          The unsigned integer <value> of the hash seed.\n");
}
