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

#include <iostream>
#include <string>
#include <map>

#include "Args.h"
#include "Path.h"
#include "StringUtils.h"
#include "Hash.h"

#include "SoundCompiler.h"
#include "TGACompiler.h"
#include "DAECompiler.h"

using namespace crown;
using std::cout;
using std::endl;
using std::ofstream;
using std::map;

// Max number of requests per run
const uint32_t	MAX_COMPILE_REQUESTS = 512;
const char*		root_path = NULL;
const char*		dest_path = NULL;
uint32_t		hash_seed = 0;

//-----------------------------------------------------------------------------
static void print_help_message(const char* program_name)
{
	cout << "Usage: " << program_name << " [options] [resources]" << endl;
	cout <<
		"Options:\n\n"

		"  --help                  Show this help.\n"
		"  --root-path <path>      The absolute <path> whether to look for the input resources.\n"
		"  --dest-path <path>      The absolute <path> whether to put the compiled resources.\n"
		"  --seed <value>          The seed to use for generating output resource hashes.\n";
}

//-----------------------------------------------------------------------------
static int32_t parse_command_line(int argc, char* argv[])
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
static void check_arguments(const char* root_path, const char* dest_path)
{
	if (root_path == NULL)
	{
		cout << "you have to specify the root path with `--root-path`" << endl;
		exit(EXIT_FAILURE);
	}

	if (dest_path == NULL)
	{
		cout << "you have to specify the destination path with `--dest-path`" << endl;
		exit(EXIT_FAILURE);
	}
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	int32_t first_resource = parse_command_line(argc, argv);

	// Check if all the mandatory options are set
	check_arguments(root_path, dest_path);

	// If there are no resources
	if (first_resource >= argc)
	{
		cout << "You have to specify at least one resource." << endl;
		exit(EXIT_FAILURE);
	}

	// Register compilers
	TGACompiler tga;
	SoundCompiler sound;
	DAECompiler dae;

	map<std::string, Compiler*> compilers;

	compilers["sound"] = &sound;
	compilers["tga"] = &tga;
	compilers["dae"] = &dae;

	for (int32_t i = 0; i < argc - first_resource; i++)
	{
		const char* resource = argv[first_resource + i];

		char resource_name[1024];
		char resource_type[1024];
		path::filename_without_extension(resource, resource_name, 1024);
		path::extension(resource, resource_type, 1024);

		uint32_t resource_name_hash = hash::murmur2_32(resource_name, string::strlen(resource_name), hash_seed);
		uint32_t resource_type_hash = hash::murmur2_32(resource_type, string::strlen(resource_type), 0);

		char out_name[1024];
		snprintf(out_name, 1024, "%.8X%.8X", resource_name_hash, resource_type_hash);

		cout << out_name << " <= " << resource << endl;

		map<std::string, Compiler*>::iterator it = compilers.find(resource_type);
		if (it != compilers.end())
		{
			if (!it->second->compile(root_path, dest_path, resource, out_name))
			{
				cout << "Exiting." << endl;
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			cout << "No compilers found for type '" << resource_type << "'." << endl;
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
