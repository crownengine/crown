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

#include "TextureCompiler.h"
#include "PixelFormat.h"
#include "Allocator.h"
#include "Filesystem.h"

namespace crown
{

//-----------------------------------------------------------------------------
TextureCompiler::TextureCompiler() :
	m_texture_data_size(0),
	m_texture_data(NULL)
{
}

//-----------------------------------------------------------------------------
TextureCompiler::~TextureCompiler()
{
}

//-----------------------------------------------------------------------------
size_t TextureCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	File* in_file = fs.open(resource_path, FOM_READ);

	if (!in_file)
	{
		Log::e("Unable to open file: %s", resource_path);
		return 0;
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
			Log::e("Unable to determine TGA channels.");
			return 0;
		}
	}

	// Determine image type (compressed/uncompressed) and call proper function to load TGA
	switch (m_tga_header.image_type)
	{
		case 0:
		{
			Log::e("The file does not contain image data: %s", resource_path);
			return 0;
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
			Log::e("Image type not supported.");
			return 0;
		}
	}

	fs.close(in_file);

	// Return the total resource size
	return sizeof(TextureHeader) + m_texture_data_size;
}

//-----------------------------------------------------------------------------
void TextureCompiler::write_impl(File* out_file)
{
	out_file->write((char*)&m_texture_header, sizeof(TextureHeader));
	out_file->write((char*)m_texture_data, m_texture_data_size);

	if (m_texture_data)
	{
		default_allocator().deallocate(m_texture_data);
		m_texture_data_size = 0;
		m_texture_data = NULL;
	}
}

//-----------------------------------------------------------------------------
void TextureCompiler::load_uncompressed(File* in_file)
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
void TextureCompiler::load_compressed(File* in_file)
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
void TextureCompiler::swap_red_blue()
{
	for (uint64_t i = 0; i < m_tga_size * m_tga_channels; i += m_tga_channels)
	{
		m_texture_data[i + 0] ^= m_texture_data[i + 2];
		m_texture_data[i + 2] ^= m_texture_data[i + 0];
		m_texture_data[i + 0] ^= m_texture_data[i + 2];
	}
}

} // namespace crown
