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

#include "AndroidMountPoint.h"
#include "AndroidFile.h"
#include "StringUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
AndroidMountPoint::AndroidMountPoint() :
	MountPoint(ANDROID_TYPE)
{
}

//-----------------------------------------------------------------------------
File* AndroidMountPoint::open(const char* relative_path, FileOpenMode /*mode*/)
{
	CE_ASSERT(exists(relative_path), "File does not exist: %s", relative_path);
	CE_ASSERT(is_file(relative_path), "File is not a regular file: %s", relative_path);

	Log::i("path: %s", relative_path);

	return CE_NEW(m_allocator, AndroidFile)(relative_path);
}

//-----------------------------------------------------------------------------
void AndroidMountPoint::close(File* file)
{
	CE_DELETE(m_allocator, file);
}

//-----------------------------------------------------------------------------
const char* AndroidMountPoint::root_path()
{
	return "Assets Folder";
}

//-----------------------------------------------------------------------------
bool AndroidMountPoint::exists(const char* relative_path)
{
	// MountPointEntry info;

	// return get_info(relative_path, info);

	return true;
}

//-----------------------------------------------------------------------------
bool AndroidMountPoint::get_info(const char* relative_path, MountPointEntry& info)
{
	// Entering OS-DEPENDENT-PATH-MODE (Android assets folder)

	// const char* os_path = relative_path;

	// string::strncpy(info.os_path, "", MAX_PATH_LENGTH);
	// string::strncpy(info.relative_path, relative_path, MAX_PATH_LENGTH);

	// AAssetDir* root = AAssetManager_openDir(g_android_asset_manager, "");

	// char asset_name[512];

	// string::strncpy(asset_name, AAssetDir_getNextFileName(root), 512);
 // 	Log::i("AssetManager: %s", asset_name);

 	return true;

	// while (asset_name != NULL)
	// {
	// 	Log::i("AssetManager: %s", asset_name);
	// 	if (string::strcmp(asset_name, relative_path) == 0)
	// 	{
	// 		info.type = MountPointEntry::FILE;
	// 		AAssetDir_rewind(root);
	// 		return true;			
	// 	}

	// 	string::strncpy(asset_name, AAssetDir_getNextFileName(root), 512);
	// }
	
	// info.type = MountPointEntry::UNKNOWN;

	// return false;
}

//-----------------------------------------------------------------------------
bool AndroidMountPoint::is_file(const char* relative_path)
{
	// MountPointEntry info;

	// if (get_info(relative_path, info))
	// {
	// 	return info.type == MountPointEntry::FILE;
	// }

	// return false;

	return true;
}

//-----------------------------------------------------------------------------
bool AndroidMountPoint::is_dir(const char* relative_path)
{
	MountPointEntry info;

	if (get_info(relative_path, info))
	{
		return info.type == MountPointEntry::DIRECTORY;
	}

	return false;
}


} // namespace crown