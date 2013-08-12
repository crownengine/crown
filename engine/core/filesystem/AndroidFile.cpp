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

#include "AssetFile.h"

namespace crown
{

//-----------------------------------------------------------------------------
AssetFile::AssetFile(const char* path) :
	File(FOM_READ),
	m_file(path, FOM_READ),
	m_last_was_read(true)
{

}

//-----------------------------------------------------------------------------
void AssetFile::seek(size_t position)
{
	check_valid();

	m_file.seek(position);
}

//-----------------------------------------------------------------------------
void AssetFile::seek_to_end()
{
	check_valid();

	m_file.seek_to_end();
}

//-----------------------------------------------------------------------------
void AssetFile::skip(size_t bytes)
{
	check_valid();

	m_file.skip(bytes);
}

//-----------------------------------------------------------------------------
void AssetFile::read(void* buffer, size_t size)
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file.seek(0);
	}

	size_t bytes_read = m_file.read(buffer, size);
	CE_ASSERT(bytes_read == size, "Failed to read from file");
}

//-----------------------------------------------------------------------------
void AssetFile::write(const void* buffer, size_t size)
{
	// Not needed
}

//-----------------------------------------------------------------------------
bool AssetFile::copy_to(File& file, size_t size = 0)
{
	// Not needed
}

//-----------------------------------------------------------------------------
void AssetFile::flush()
{
	// Not needed
}

//-----------------------------------------------------------------------------
bool AssetFile::is_valid() const
{
	return m_file.is_open();
}

//-----------------------------------------------------------------------------
bool AssetFile::end_of_file() const
{
	return position() == size();
}

//-----------------------------------------------------------------------------
size_t AssetFile::size() const
{
	check_valid();

	return m_file.size();
}

//-----------------------------------------------------------------------------
size_t AssetFile::position() const
{
	check_valid();

	return m_file.position();
}

//-----------------------------------------------------------------------------
bool AssetFile::can_read() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool AssetFile::can_write() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool AssetFile::can_seek() const
{
	check_valid();

	return true;
}

} // namespace crown