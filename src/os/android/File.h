/*
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

#include "Types.h"
#include <sys/types.h>
#include <android/asset_manager.h>

namespace crown
{

/**
	Enumerates file opening modes.
*/
enum FileOpenMode
{
	FOM_READ	= 1,
	FOM_WRITE	= 2
};

/**
	Standard C file wrapper.
*/
class File
{

public:

						~File();

	bool				is_valid();

	FileOpenMode		mode();

	size_t				read(void* ptr, size_t size, size_t nmemb);
	size_t				write(const void* ptr, size_t size, size_t nmemb);
	int					seek(int32_t offset, int whence);
	int32_t				tell();

	int					eof();

	size_t				size();

	static File*		open(const char* path, FileOpenMode mode);

private:

	AAsset*				m_asset;
	FileOpenMode		m_mode;

						File();
};

} // namespace crown

