#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Crown.h"
#include "LuaCompiler.h"

using namespace crown;

const char* root_path = NULL;
const char* dest_path = NULL;
const char* resource_in = NULL;
uint32_t hash_seed = 0;

void 	parse_command_line(int argc, char** argv);
void 	print_help_message(const char* program_name);
void	compile_script(char* tmp_out);

/// Lua scripts compiler
int main(int argc, char** argv)
{
	parse_command_line(argc, argv);
	
	if (root_path == NULL)
	{
		printf("%s: ERROR: you have to specify the root path with `--root-path`\n", argv[0]);
		exit(-1);
	}

	if (dest_path == NULL)
	{
		printf("%s: ERROR: you have to specify the destination path with `--dest-path`\n", argv[0]);
		exit(-1);
	}

	if (resource_in == NULL)
	{
		printf("%s: ERROR: you have to specify the resource name with `--resource-in`\n", argv[0]);
		exit(-1);
	}

	LuaCompiler compiler(root_path, dest_path, resource_in, hash_seed);

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

void print_help_message(const char* program_name)
{
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n\n");

	printf("  --help                  Show this help.\n");
	printf("  --root-path <path>      The _absolute_ <path> whether to look for the input resource.\n");
	printf("  --resource-in <path>    The _relative_ <path> of the input resource.\n");
	printf("  --resource-out <width>  The _relative_ <path> of the output resource.\n");
}

void compile_script(char* tmp_out)
{
	char in[256];
	strcpy(in, root_path);
	strcat(in, resource_in);

	char rel_out[256];
	strncpy(rel_out, resource_in, strlen(resource_in) - 3);
	strcat(rel_out, "tmp");

	char out[256];
	strcpy(out, root_path);
	strcat(out, rel_out);

	strcpy(tmp_out, rel_out);

	// Fork for execl
	pid_t child = 0;

	child = fork();

	if (child < 0)
	{
		printf("Failed fork during compile_script() call.");
		return;
	}

	if (child == 0)	
	{
		wait(NULL);
	}
	else
	{
		execl("/usr/local/bin/luajit", "luajit", "-bl", in, out, NULL);
	}
}
