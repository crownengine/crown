#include <stdio.h>
#include <stdlib.h>

#include "Crown.h"

const char* root_path = NULL;
const char* resource_in = NULL;
const char* resource_out = NULL;

void print_help_message(const char* program_name);
void parse_command_line(int argc, char** argv);

using namespace crown;

struct TGAHeader
{

	char		id_length;			// 00h  Size of Image ID field
	char		color_map_type;		// 01h  Color map type
	char		image_type;			// 02h  Image type code
	char		c_map_spec[5];		// 03h  Color map origin 05h Color map length 07h Depth of color map entries
	uint16_t	x_offset;			// 08h  X origin of image
	uint16_t	y_offset;			// 0Ah  Y origin of image
	uint16_t	width;				// 0Ch  Width of image
	uint16_t	height;				// 0Eh  Height of image
	char		pixel_depth;     	// 10h  Image pixel size
	char		image_descriptor;	// 11h  Image descriptor byte
};

void print_help_message(const char* program_name);
void parse_command_line(int argc, char** argv);

void load_uncompressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels);
void load_compressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels);
void swap_red_blue(uint8_t* data, uint64_t size, uint32_t channels);

/// TGA compiler for "tga" resource type
/// TODO: Explain supported formats, usage etc.
int main(int argc, char** argv)
{
	parse_command_line(argc, argv);
	
	// FIXME: validate input

	Filesystem fs_root(root_path);
	
	if (!fs_root.exists(resource_in))
	{
		printf("%s: ERROR: %s does not exist. Aborting.\n", argv[0], resource_in);
		return -1;
	}
	
	char resource_basename[256];
	char resource_extension[256];
	
	path::filename_without_extension(resource_in, resource_basename, 256);
	path::extension(resource_in, resource_extension, 256);
	
	uint32_t resource_basename_hash = hash::fnv1a_32(resource_basename, string::strlen(resource_basename));
	uint32_t resource_extension_hash = hash::fnv1a_32(resource_extension, string::strlen(resource_extension));

	FileStream* src_file = (FileStream*)fs_root.open(resource_in, SOM_READ);
	
	//-------------------------------------------------------------------------
	// Read TGA Header
	//-------------------------------------------------------------------------
	
	// The TGA header used throughout the code
	TGAHeader header;
	memset(&header, 0, sizeof(TGAHeader));
	
	// Read the header
	src_file->read(&header, sizeof(TGAHeader));

	// Skip TGA ID
	src_file->skip(header.id_length);

	// Pixel format currently unknown
	PixelFormat format = PF_UNKNOWN;

	// Compute color channels	
	uint32_t channels = header.pixel_depth / 8;
	
	// Compute image size
	uint64_t image_size = header.width * header.height;
	
	uint8_t* image_data = NULL;

	// Select the appropriate pixel format and allocate resource data based on tga size and channels
	switch (channels)
	{
		case 2:
		case 3:
		{
			format = PF_RGB_8;
			image_data = new uint8_t[(uint32_t)(image_size * 3)];
			
			break;
		}
		case 4:
		{
			format = PF_RGBA_8;
			image_data = new uint8_t[(uint32_t)(image_size * channels)];
			
			break;
		}
		default:
		{
			printf("Fatal: Unable to determine TGA channels. Aborting.\n");
			return -1;
		}
	}

	// Determine image type (compressed/uncompressed) and call proper function to load TGA
	switch (header.image_type)
	{
		case 0:
		{
			printf("Fatal: The resource does not contain image data. Aborting.");
			exit(-1);
		}
		case 2:
		{
			load_uncompressed(image_data, src_file, header.width, header.height, channels);
			break;
		}

		case 10:
		{
			load_compressed(image_data, src_file, header.width, header.height, channels);
			break;
		}

		default:
		{
			printf("Fatal: Image type not supported. Aborting.");
			exit(-1);
		}
	}
	
	// Open output file
	FileStream* dest_file = (FileStream*)fs_root.open(resource_out, SOM_WRITE);
	
	ArchiveEntry archive_entry;
	archive_entry.name = resource_basename_hash;
	archive_entry.type = resource_extension_hash;
	archive_entry.offset = sizeof(ArchiveEntry);
	archive_entry.size = image_size * channels + sizeof(PixelFormat) + sizeof(uint16_t) * 2;
							
	// Write out the archive entry
	dest_file->write(&archive_entry, sizeof(ArchiveEntry));

	// Write out the data
	dest_file->write(&format, sizeof(PixelFormat));
	dest_file->write(&header.width, sizeof(uint16_t));
	dest_file->write(&header.height, sizeof(uint16_t));
	
	dest_file->write(image_data, image_size * channels);
	
	// Done, free the resources and exit
	if (image_data != NULL)
	{
		delete[] image_data;
	}
	
	fs_root.close(dest_file);

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

//-----------------------------------------------------------------------------
void print_help_message(const char* program_name)
{
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n\n");

	printf("  --help                  Show this help.\n");
	printf("  --root-path <path>      The _absolute_ <path> whether to look for the input resource.\n");
	printf("  --resource-in <path>    The _relative_ <path> of the input resource.\n");
	printf("  --resource-out <width>  The _relative_ <path> of the output resource.\n");
}

//-----------------------------------------------------------------------------
void load_uncompressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels)
{
	uint64_t size = width * height;
	
	uint8_t* data = (uint8_t*)dest;

	if (channels == 2)
	{
		int32_t j = 0;

		for (uint64_t i = 0; i < size * channels; i++)
		{
			uint16_t pixel_data;
			
			stream->read(&pixel_data, sizeof(pixel_data));
			
			data[j + 0] = (pixel_data & 0x7c) >> 10;
			data[j + 1] = (pixel_data & 0x3e) >> 5;
			data[j + 2] = (pixel_data & 0x1f);
			
			j += 3;
		}
	}
	else
	{
		stream->read(data, (size_t)(size * channels));

		swap_red_blue(data, size * channels, channels);
	}
}

//-----------------------------------------------------------------------------
void load_compressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels)
{
	uint8_t rle_id = 0;
	uint32_t i = 0;
	uint32_t colors_read = 0;
	uint64_t size = width * height;
	
	uint8_t* data = (uint8_t*)dest;

	uint8_t* colors = new uint8_t[channels];

	while (i < size)
	{
		stream->read(&rle_id, sizeof(uint8_t));

		// If MSB == 1
		if (rle_id & 0x80)
		{
			rle_id -= 127;
			
			stream->read(colors, channels);

			while (rle_id)
			{
				data[colors_read + 0] = colors[2];
				data[colors_read + 1] = colors[1];
				data[colors_read + 2] = colors[0];

				if (channels == 4)
				{
					data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
		else
		{
			rle_id++;

			while (rle_id)
			{
				stream->read(colors, channels);
				
				data[colors_read + 0] = colors[2];
				data[colors_read + 1] = colors[1];
				data[colors_read + 2] = colors[0];

				if (channels == 4)
				{
					data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
	}

	delete[] colors;
}

//-----------------------------------------------------------------------------
void swap_red_blue(uint8_t* data, uint64_t size, uint32_t channels)
{
	for (uint64_t i = 0; i < size; i += channels)
	{
		data[i] ^= data[i+2];
		data[i+2] ^= data[i];
		data[i] ^= data[i+2];
	}
}

