#include "Crown.h"
#include "tga/TGACompiler.h"
#include "txt/TXTCompiler.h"
#include "lua/LuaCompiler.h"
#include "vs/VSCompiler.h"
#include "ps/PSCompiler.h"

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
		Log::e("you have to specify at least one resource.");
		exit(EXIT_FAILURE);
	}

	TGACompiler tga(root_path, dest_path);
	TXTCompiler txt(root_path, dest_path);
	LuaCompiler lua(root_path, dest_path);
	VSCompiler vs(root_path, dest_path);
	PSCompiler ps(root_path, dest_path);

	char resource_name[MAX_RESOURCE_NAME_LENGTH];
	char resource_type[MAX_RESOURCE_TYPE_LENGTH];

	// Dispatch requests to the appropriate compiler
	for (int32_t res = first_resource; res < argc; res++)
	{
		char* resource = argv[res];

		path::filename_without_extension(resource, resource_name, MAX_RESOURCE_NAME_LENGTH);
		path::extension(resource, resource_type, MAX_RESOURCE_TYPE_LENGTH);

		uint32_t resource_name_hash = hash::murmur2_32(resource_name, string::strlen(resource_name), hash_seed);
		uint32_t resource_type_hash = hash::murmur2_32(resource_type, string::strlen(resource_type), 0);

		switch (resource_type_hash)
		{
			case TEXTURE_TYPE:
			{
				tga.compile(resource, resource_name_hash, resource_type_hash);
				break;
			}
			case TEXT_TYPE:
			{
				txt.compile(resource, resource_name_hash, resource_type_hash);
				break;
			}
			case SCRIPT_TYPE:
			{
				lua.compile(resource, resource_name_hash, resource_type_hash);
				break;
			}
			case VERTEX_SHADER_TYPE:
			{
				vs.compile(resource, resource_name_hash, resource_type_hash);
				break;
			}
			case PIXEL_SHADER_TYPE:
			{
				ps.compile(resource, resource_name_hash, resource_type_hash);
				break;	
			}
			default:
			{
				Log::e("Resource type not supported.");
				break;
			}
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
		Log::e("you have to specify the root path with `--root-path`\n");
		exit(EXIT_FAILURE);
	}

	if (dest_path == NULL)
	{
		Log::e("you have to specify the destination path with `--dest-path`\n");
		exit(EXIT_FAILURE);
	}
}
