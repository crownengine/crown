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

#include <cstdio>

#include "Args.h"
#include "Path.h"
#include "StringUtils.h"
#include "Hash.h"
#include "TGACompiler.h"
#include "WAVCompiler.h"

using namespace crown;

// Max number of requests per run
const uint32_t MAX_COMPILE_REQUESTS = 512;

const char*		root_path = NULL;
const char*		dest_path = NULL;
uint32_t		hash_seed = 0;

// Help functions
int32_t			parse_command_line(int argc, char* argv[]);
void			print_help_message(const char* program_name);
void			check_arguments(const char* root_path, const char* dest_path);
void			compile_by_type(const char* resource);

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	int32_t first_resource = parse_command_line(argc, argv);

	// Check if all the mandatory options are set
	check_arguments(root_path, dest_path);

	// If there are no resources
	if (first_resource >= argc)
	{
		printf("you have to specify at least one resource.");
		exit(EXIT_FAILURE);
	}


	TGACompiler tga;
	WAVCompiler wav;

	char out_name[1024];
	char resource_name[1024];
	char resource_type[1024];

	for (int32_t i = 0; i < argc - first_resource; i++)
	{
		path::filename_without_extension(argv[first_resource + i], resource_name, 1024);
		path::extension(argv[first_resource + i], resource_type, 1024);

		snprintf(out_name, 1024, "%.8X%.8X",
			hash::murmur2_32(resource_name, string::strlen(resource_name), hash_seed),
			hash::murmur2_32(resource_type, string::strlen(resource_type), 0));

		printf("%s <= %s\n", out_name, argv[first_resource + i]);

		if (string::strcmp(resource_type, "tga") == 0)
		{
			tga.compile(root_path, dest_path, argv[first_resource + i], out_name);
		}
		else if (string::strcmp(resource_type, "wav") == 0)
		{
			wav.compile(root_path, dest_path, argv[first_resource + i], out_name);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
int32_t parse_command_line(int argc, char* argv[])
{
	// Parse arguments
	static ArgsOption options[] = 
	{
		"help",         AOA_NO_ARGUMENT,       NULL,        'h',
		"root-path",    AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"dest-path",    AOA_REQUIRED_ARGUMENT, NULL,        'd',
		"seed",         AOA_REQUIRED_ARGUMENT, NULL,        's',
		NULL, 0, NULL, 0
	};

	Args args(argc, argv, "", options);

	int32_t opt;

	while ((opt = args.getopt()) != -1)
	{
		switch (opt)
		{
			// Root path
			case 'r':
			{
				root_path = args.optarg();
				break;
			}
			// Dest path
			case 'd':
			{
				dest_path = args.optarg();
				break;
			}
			case 's':
			{
				hash_seed = atoi(args.optarg());
				break;
			}
			case 'h':
			case '?':
			default:
			{
				print_help_message(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
	}

	return args.optind();
}

//-----------------------------------------------------------------------------
void print_help_message(const char* program_name)
{
	printf("Usage: %s [options] [resources]\n", program_name);
	printf
	(
		"Options:\n\n"

		"  --help                  Show this help.\n"
		"  --root-path <path>      The absolute <path> whether to look for the input resources.\n"
		"  --dest-path <path>      The absolute <path> whether to put the compiled resources.\n"
		"  --seed <value>          The seed to use for generating output resource hashes.\n"
	);
}

//-----------------------------------------------------------------------------
void check_arguments(const char* root_path, const char* dest_path)
{
	if (root_path == NULL)
	{
		printf("you have to specify the root path with `--root-path`\n");
		exit(EXIT_FAILURE);
	}

	if (dest_path == NULL)
	{
		printf("you have to specify the destination path with `--dest-path`\n");
		exit(EXIT_FAILURE);
	}
}
