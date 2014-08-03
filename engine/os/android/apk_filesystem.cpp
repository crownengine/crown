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

#include <sys/types.h>
#include <android/asset_manager.h>
#include "apk_filesystem.h"
#include "temp_allocator.h"
#include "apk_file.h"
#include "os.h"

namespace crown
{

extern AAssetManager* get_android_asset_manager();

//-----------------------------------------------------------------------------
ApkFilesystem::ApkFilesystem()
{
}

//-----------------------------------------------------------------------------
File* ApkFilesystem::open(const char* path, FileOpenMode mode)
{
	CE_ASSERT_NOT_NULL(path);
	CE_ASSERT(mode == FOM_READ, "Cannot open for writing in Android assets folder");

	return CE_NEW(default_allocator(), ApkFile)(path);
}

//-----------------------------------------------------------------------------
void ApkFilesystem::close(File* file)
{
	CE_ASSERT_NOT_NULL(file);
	CE_DELETE(default_allocator(), file);
}

//-----------------------------------------------------------------------------
bool ApkFilesystem::is_directory(const char* path)
{
	return true;
}

//-----------------------------------------------------------------------------
bool ApkFilesystem::is_file(const char* path)
{
	return true;
}

//-----------------------------------------------------------------------------
void ApkFilesystem::create_directory(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to create directory in Android assets folder");
}

//-----------------------------------------------------------------------------
void ApkFilesystem::delete_directory(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to delete directory in Android assets folder");
}

//-----------------------------------------------------------------------------
void ApkFilesystem::create_file(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to create file in Android assets folder");
}

//-----------------------------------------------------------------------------
void ApkFilesystem::delete_file(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to delete file in Android assets folder");
}

//-----------------------------------------------------------------------------
void ApkFilesystem::list_files(const char* path, Vector<DynamicString>& files)
{
	CE_ASSERT_NOT_NULL(path);

	AAssetDir* root_dir = AAssetManager_openDir(get_android_asset_manager(), path);
	CE_ASSERT(root_dir != NULL, "Failed to open Android assets folder");

	const char* filename = NULL;
	while ((filename = AAssetDir_getNextFileName(root_dir)) != NULL)
	{
		DynamicString name(default_allocator());
		name = filename;
		vector::push_back(files, name);
	}

	AAssetDir_close(root_dir);
}

//-----------------------------------------------------------------------------
void ApkFilesystem::get_absolute_path(const char* path, DynamicString& os_path)
{
	os_path = path;
}

} // namespace crown
