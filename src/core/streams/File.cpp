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

#include "File.h"
#include "Log.h"
#include "MathUtils.h"

namespace Crown
{

File::File() :
	mFileHandle(0), mMode(FOM_READ)
{
}

File::~File()
{
	if (mFileHandle != 0)
	{
		fclose(mFileHandle);
	}
}

bool File::IsValid()
{
	return mFileHandle != 0;
}

FileOpenMode File::GetMode()
{
	return mMode;
}

FILE* File::GetHandle()
{
	return mFileHandle;
}

File* File::Open(const char* path, FileOpenMode mode)
{
	File* f = new File();
	f->mFileHandle = fopen(path, 

	/*
		TestFlag(mode, FOM_READ) ?
			(TestFlag(mode, FOM_WRITE) ?
				(TestFlag(mode, FOM_CREATENEW) ? "wb+" : "rb+") : "rb") : (TestFlag(mode, FOM_WRITE) ? "wb" : "rb"));
	*/

	Math::TestBitmask(mode, FOM_READ) ?
		(Math::TestBitmask(mode, FOM_WRITE) ? "rb+" : "rb") : (Math::TestBitmask(mode, FOM_WRITE) ? "wb" : "rb")); 

	if (f->mFileHandle == 0)
	{
		Log::E("File::Open: Could not open file %s", path);
		return NULL;
	}

	f->mMode = mode;

	return f;
}

size_t File::GetSize()
{
	size_t pos = ftell(mFileHandle);
	fseek(mFileHandle, 0, SEEK_END);
	size_t size = ftell(mFileHandle);
	fseek(mFileHandle, pos, SEEK_SET);

	return size;
}

} // namespace Crown

