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

#include "Allocator.h"
#include "Filesystem.h"
#include "PixelFormat.h"
#include "TextureResource.h"

namespace crown
{
namespace texture_resource
{

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

TGAHeader		m_tga_header;
uint32_t		m_tga_channels;
uint32_t		m_tga_size;

TextureHeader	m_texture_header;
size_t			m_texture_data_size = 0;
uint8_t*		m_texture_data = NULL;

//-----------------------------------------------------------------------------
void swap_red_blue()
{
	for (uint64_t i = 0; i < m_tga_size * m_tga_channels; i += m_tga_channels)
	{
		m_texture_data[i + 0] ^= m_texture_data[i + 2];
		m_texture_data[i + 2] ^= m_texture_data[i + 0];
		m_texture_data[i + 0] ^= m_texture_data[i + 2];
	}
}

//-----------------------------------------------------------------------------
void load_uncompressed(File* in_file)
{
	uint64_t size = m_tga_header.width * m_tga_header.height;

	if (m_tga_channels == 2)
	{
		int32_t j = 0;

		for (uint64_t i = 0; i < size * m_tga_channels; i++)
		{
			uint16_t pixel_data;
			
			in_file->read((char*)&pixel_data, sizeof(pixel_data));
			
			m_texture_data[j + 0] = (pixel_data & 0x7c) >> 10;
			m_texture_data[j + 1] = (pixel_data & 0x3e) >> 5;
			m_texture_data[j + 2] = (pixel_data & 0x1f);
			
			j += 3;
		}
	}
	else
	{
		in_file->read((char*)m_texture_data, (size_t)(size * m_tga_channels));

		swap_red_blue();
	}
}

//-----------------------------------------------------------------------------
void load_compressed(File* in_file)
{
	uint8_t rle_id = 0;
	uint32_t i = 0;
	uint32_t colors_read = 0;
	uint64_t size = m_tga_header.width * m_tga_header.height;

	// Can't be more than 4 channels
	uint8_t colors[4];

	while (i < size)
	{
		in_file->read((char*)&rle_id, sizeof(uint8_t));

		// If MSB == 1
		if (rle_id & 0x80)
		{
			rle_id -= 127;
			
			in_file->read((char*)&colors, m_tga_channels);

			while (rle_id)
			{
				m_texture_data[colors_read + 0] = colors[2];
				m_texture_data[colors_read + 1] = colors[1];
				m_texture_data[colors_read + 2] = colors[0];

				if (m_tga_channels == 4)
				{
					m_texture_data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += m_tga_channels;
				i++;
			}
		}
		else
		{
			rle_id++;

			while (rle_id)
			{
				in_file->read((char*)colors, m_tga_channels);
				
				m_texture_data[colors_read + 0] = colors[2];
				m_texture_data[colors_read + 1] = colors[1];
				m_texture_data[colors_read + 2] = colors[0];

				if (m_tga_channels == 4)
				{
					m_texture_data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += m_tga_channels;
				i++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* in_file = fs.open(resource_path, FOM_READ);

	if (!in_file)
	{
		CE_LOGE("Unable to open file: %s", resource_path);
		return;
	}

	// Read the header
	in_file->read((char*)(char*)&m_tga_header, sizeof(TGAHeader));

	// Skip TGA ID
	in_file->skip(m_tga_header.id_length);

	// Compute color channels
	m_tga_channels = m_tga_header.pixel_depth / 8;
	m_tga_size = m_tga_header.width * m_tga_header.height;

	m_texture_header.version = TEXTURE_VERSION;
	m_texture_header.width = m_tga_header.width;
	m_texture_header.height = m_tga_header.height;

	// Select the appropriate pixel format and allocate
	// resource data based on tga size and channels
	switch (m_tga_channels)
	{
		case 2:
		case 3:
		{
			m_texture_header.format = PixelFormat::RGB_8;

			m_texture_data_size = m_tga_size * 3;
			m_texture_data = (uint8_t*)default_allocator().allocate(m_texture_data_size);

			break;
		}
		case 4:
		{
			m_texture_header.format = PixelFormat::RGBA_8;

			m_texture_data_size = m_tga_size * m_tga_channels;
			m_texture_data = (uint8_t*)default_allocator().allocate(m_texture_data_size);
			
			break;
		}
		default:
		{
			CE_LOGE("Unable to determine TGA channels.");
			return;
		}
	}

	// Determine image type (compressed/uncompressed) and call proper function to load TGA
	switch (m_tga_header.image_type)
	{
		case 0:
		{
			CE_LOGE("The file does not contain image data: %s", resource_path);
			return;
		}
		case 2:
		{
			load_uncompressed(in_file);
			break;
		}

		case 10:
		{
			load_compressed(in_file);
			break;
		}

		default:
		{
			CE_LOGE("Image type not supported.");
			return;
		}
	}

	fs.close(in_file);

	out_file->write((char*)&m_texture_header, sizeof(TextureHeader));
	out_file->write((char*)m_texture_data, m_texture_data_size);

	if (m_texture_data)
	{
		default_allocator().deallocate(m_texture_data);
		m_texture_data_size = 0;
		m_texture_data = NULL;
	}
}

} // namespace texture_resource
} // namespace crown