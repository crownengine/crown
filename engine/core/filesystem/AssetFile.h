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

#pragma once

#include "APKFile.h"

namespace crown
{

class AssetFile : public File 
{
public:

				AssetFile(const char* path);

	void		seek(size_t position);

	void		seek_to_end();

	void		skip(size_t bytes);

	void		read(void* buffer, size_t size);

	void		write(const void* buffer, size_t size);

	bool		copy_to(File& file, size_t size = 0);

	void		flush();

	bool		is_valid() const;

	bool		end_of_file() const;

	size_t		size() const;

	size_t		position() const;

	bool		can_read() const;

	bool		can_write() const;

	bool		can_seek() const;

private:

	APKFile*	m_file;

	bool		m_last_was_read;

private:

	inline void		check_valid() const
	{
		CE_ASSERT(m_file.is_open(), "File is not open");
	}
};

} // namespace crown
