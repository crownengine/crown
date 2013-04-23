#include <stdio.h>
#include <stdlib.h>

#include "Crown.h"
#include "TGACompiler.h"

const char* root_path = NULL;
const char* dest_path = NULL;
const char* resource_in = NULL;
uint32_t hash_seed = 0;

using namespace crown;

void print_help_message(const char* program_name);
void parse_command_line(int argc, char** argv);

/// TGA compiler for "tga" resource type
/// TODO: Explain supported formats, usage etc.
int main(int argc, char** argv)
{
	parse_command_line(argc, argv);
	
	// FIXME: validate input

	TGACompiler compiler(root_path, dest_path, resource_in, hash_seed);

	if (compiler.compile() == false)
	{
		printf("%s: ERROR: compilation failed for resource %s\n", argv[0], compiler.resource_path());
		exit(-1);
	}

	compiler.write();

	return 0;
}

//-----------------------------------------------------------------------------
void parse_command_line(int argc, char** argv)
{
	// Parse arguments
	ArgsOption options[] = 
	{
		"help",         AOA_NO_ARGUMENT,       NULL,        'h',
		"root-path",    AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"dest-path",    AOA_REQUIRED_ARGUMENT, NULL,        'd',
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
			// Dest path
			case 'd':
			{
				if (args.option_argument() == NULL)
				{
					printf("%s: ERROR: missing path after `--dest-path`\n", argv[0]);
					exit(-1);
				}
				
				dest_path = args.option_argument();
				
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
	printf("  --root-path <path>      The _absolute_ <path> whether to look for the input resource.\n");
	printf("  --dest-path <path>      The _absolute_ <path> whether to put the compiled resource.\n");
	printf("  --resource-in <path>    The _relative_ <path> of the input resource.\n");
	printf("  --seed <value>          The unsigned integer <value> of the hash seed.\n");
}

