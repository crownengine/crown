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

#include "Filesystem.h"
#include "Log.h"
#include "OS.h"
#include "FileStream.h"

#include "Config.h"
#ifdef WINDOWS
#undef DeleteFile
#undef CreateDirectory
#endif

namespace crown
{

//-----------------------------------------------------------------------------
Filesystem::Filesystem() :
	mIsInit(false),
	mRootPath(Str::EMPTY),
	mUserPath(Str::EMPTY)
{
}

//-----------------------------------------------------------------------------
Filesystem::~Filesystem()
{
}

//-----------------------------------------------------------------------------
void Filesystem::Init(const char* rootPath, const char* userPath)
{
	assert(!mIsInit);

	if (Str::StrCmp(rootPath, Str::EMPTY) == 0)
	{
		// Set working paths
		const char* envRootPath = os::get_env("CROWN_ROOT_PATH");

		if (envRootPath == Str::EMPTY)
		{
			mRootPath = os::get_cwd();
		}
		else
		{
			mRootPath = envRootPath;
		}
	}
	else
	{
		mRootPath = rootPath;
	}

	if (userPath == Str::EMPTY)
	{
		mUserPath = os::get_home();
	}
	else
	{
		mUserPath = userPath;
	}

	Log::D("Filesystem::Init()");
	Log::D("Filesystem: Root path: %s", mRootPath.c_str());
	Log::D("Filesystem: User path: %s", mUserPath.c_str());

	mIsInit = true;
}

//-----------------------------------------------------------------------------
const char* Filesystem::GetRootPath() const
{
	return mRootPath.c_str();
}

//-----------------------------------------------------------------------------
const char* Filesystem::GetUserPath() const
{
	return mUserPath.c_str();
}

//-----------------------------------------------------------------------------
void Filesystem::SetUserPath(const char* relativePath)
{
	mUserPath = relativePath;
}

//-----------------------------------------------------------------------------
const char* Filesystem::BuildOSPath(const char* basePath, const char* relativePath)
{
	static char osPath[1024];

	size_t i = 0;

	while (*basePath != '\0')
	{
		osPath[i++] = *basePath;
		basePath++;
	}

	osPath[i++] = '/';

	while (*relativePath != '\0')
	{
		osPath[i++] = *relativePath;
		relativePath++;
	}

	osPath[i] = '\0';

	// Replace Crown-specific path separator with OS-speficic one
	for (size_t j = 0; j < i; j++)
	{
		if (osPath[j] == '/')
		{
			osPath[j] = os::PATH_SEPARATOR;
		}
	}

	return osPath;
}

//-----------------------------------------------------------------------------
bool Filesystem::GetInfo(const char* basePath, const char* relativePath, FilesystemEntry& info)
{
	// Entering OS-DEPENDENT-PATH-MODE
	// (i.e. osPath is of the form: C:\babbeo\relativePath or /babbeo/relativePath)

	const char* osPath = BuildOSPath(basePath, relativePath);

	if (os::is_reg(osPath))
	{
		info.type			= FET_FILE;
		info.osPath			= osPath;
		info.relativePath	= relativePath;
		return true;
	}

	if (os::is_dir(osPath))
	{
		info.type			= FET_DIR;
		info.osPath			= osPath;
		info.relativePath	= relativePath;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::Exists(const char* relativePath)
{
	FilesystemEntry dummy;

	return GetInfo(mRootPath.c_str(), relativePath, dummy);
}

//-----------------------------------------------------------------------------
bool Filesystem::IsFile(const char* relativePath)
{
	FilesystemEntry info;

	if (GetInfo(mRootPath.c_str(), relativePath, info))
	{
		return info.type == FET_FILE;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::IsDir(const char* relativePath)
{
	FilesystemEntry info;

	if (GetInfo(mRootPath.c_str(), relativePath, info))
	{
		return info.type == FET_DIR;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::CreateFile(const char* relativePath)
{
	const char* osPath = BuildOSPath(mRootPath.c_str(), relativePath);

	return os::mknod(osPath);
}

//-----------------------------------------------------------------------------
bool Filesystem::CreateDir(const char* relativePath)
{
	const char* osPath = BuildOSPath(mRootPath.c_str(), relativePath);

	return os::mkdir(osPath);
}

//-----------------------------------------------------------------------------
bool Filesystem::DeleteFile(const char* relativePath)
{
	const char* osPath = BuildOSPath(mRootPath.c_str(), relativePath);

	return os::unlink(osPath);
}

//-----------------------------------------------------------------------------
bool Filesystem::DeleteDir(const char* relativePath)
{
	const char* osPath = BuildOSPath(mRootPath.c_str(), relativePath);

	return os::rmdir(osPath);
}

//-----------------------------------------------------------------------------
Stream* Filesystem::OpenStream(const char* relativePath, StreamOpenMode openMode)
{
	FilesystemEntry info;
	Stream* stream;

	bool exists = GetInfo(mRootPath.c_str(), relativePath, info);

	assert(exists == true && "Filesystem::OpenStream: File does not exist");
	assert(info.type != FET_DIR && "Filesystem::OpenStream: Trying to open directory...");

	if (info.type == FET_FILE)
	{
		Log::D("Filesystem::OpenStream: Found %s", info.osPath.c_str());

		stream = new FileStream(openMode, info.osPath);

		return stream;
	}

	return NULL;
}

/**
	file must exists.
*/
Stream* Filesystem::OpenRead(const char* relativePath)
{
	(void)relativePath;
	return NULL;
}

Stream* Filesystem::OperWrite(const char* relativePath)
{
	(void)relativePath;
	return NULL;
}

Stream* Filesystem::OpenAppend(const char* relativePath)
{
	(void)relativePath;
	return NULL;
}

//-----------------------------------------------------------------------------
void Filesystem::Close(Stream* stream)
{
	delete stream;
}

//-----------------------------------------------------------------------------
void Filesystem::Print32_tReport()
{
	Log::I("--- Filesystem report ---");
	Log::I("Path separator\t: '/'");
	Log::I("Root\t\t\t: %s", mRootPath.c_str());
	Log::I("User\t\t\t: %s", mUserPath.c_str());
}

///**
//	Returns whether the segment is valid.
//@note
//	The rules for valid segments are as follows:
//	a) The empty string is not valid.
//	b) Any string containing the slash character ('/') is not valid.
//	c) Common notations for current ('.') and parent ('..') directory are forbidden.
//	d) Any string containing segment or device separator characters on the local file system,
//	such as the backslash ('\') and colon (':') on some file systems.
//	(Thanks org.eclipse.core.runtime for the documentation ;D).
//@param segment
//	The segment to be checked
//@return
//	True if the segment is valid, false otherwise
//*/
//bool Filesystem::IsValidSegment(const char* segment)
//{
//	size_t segmentLen = Str::StrLen(segment);

//	if (segmentLen == 0)
//	{
//		return false;
//	}

//	if (segmentLen == 1 && segment[0] == '.')
//	{
//		return false;
//	}

//	if (segmentLen == 2 && segment[0] == '.' && segment[1] == '.')
//	{
//		return false;
//	}

//	for (size_t i = 0; i < segmentLen; i++)
//	{
//		if (segment[i] == '/' ||
//			segment[i] == '\\' ||
//			segment[i] == ':'
//		{
//			return false;
//		}
//	}

//	return true;
//}

///**
//	Returns whether the path is valid.
//@note
//	The rules for valid paths are as follows:
//	a) The empty string is not valid.
//	b) If the path is absolute, it mustn't contain any leading character.
//@param path
//	The path to be checked
//@return
//	True if the path is valid, false otherwise
//*/
//bool Filesystem::IsValidPath(const char* path)
//{
//	size_t pathLen = Str::StrLen(path);

//	if (pathLen == 0)
//	{
//		return false;
//	}

//	if (IsRootPath(path))
//	{
//		return true;
//	}

//	List<Str> segmentList;
//	if (!GetSegments(Str(path), segmentList))
//	{
//		return false;
//	}

//	size_t i = 0;
//	if (IsAbsolutePath(path) && path[0] != '/')
//	{
//		i = 1;
//	}

//	for (; i < segmentList.GetSize(); i++)
//	{
//		if (!IsValidSegment(segmentList[i].c_str()))
//		{
//			return false;
//		}
//	}

//	return true;
//}

///**
//	Returns whether the path is absolute.
//@note
//	(i.e. starts with Path::SEPARATOR or <a-Z><Path::DEVICE_SEPARATOR><Path::SEPARATOR>).
//@param path
//	The path to be checked
//@return
//	True if absolute, false otherwise
//*/
//bool Filesystem::IsAbsolutePath(const char* path)
//{
//	size_t pathLen;

//	pathLen = Str::StrLen(path);

//	if (pathLen == 0)
//	{
//		return false;
//	}

//	if (path[0] == '/')
//	{
//		return true;
//	}

//	if (pathLen < 3)
//	{
//		return false;
//	}

//	if (Str::IsAlpha(path[0]) && path[1] == ':' && path[2] == os::PATH_SEPARATOR)
//	{
//		return true;
//	}

//	return false;
//}

///**
//	Returns whether the path is the root path.
//@note
//	(i.e. starts and ends with Path::SEPARATOR or <a-Z><Path::DEVICE_SEPARATOR><Path::SEPARATOR>).
//@param path
//	The path to be checked
//@return
//	True if root, false otherwise
//*/
//bool Filesystem::IsRootPath(const char* path)
//{
//	size_t pathLen;

//	pathLen = Str::StrLen(path);

//	if (pathLen == 0)
//	{
//		return false;
//	}

//	if (pathLen == 1 && path[0] == PATH_SEPARATOR)
//	{
//		return true;
//	}

//	if (pathLen == 3 && Str::IsAlpha(path[0]) && path[1] == DEVICE_SEPARATOR && path[2] == PATH_SEPARATOR)
//	{
//		return true;
//	}

//	return false;
//}

///**
//	Returns the pathname of the path.
//@note
//	(e.g. /home/babbeo/texture.tga -> /home/babbeo).
//@param path
//	The input path
//@param ret
//	The output pathname
//@return
//	True if success, false otherwise
//*/
//const char* Filesystem::GetPathname(const char* path)
//{
//	size_t pathLen;

//	pathLen = Str::StrLen(path);

//	const char* c = path + pathLen - 1;

//	// Ignore any trailing separators
//	while (c > path && *c == PATH_SEPARATOR)
//	{
//		c--;
//	}

//	// Iterate backwards until first separator
//	while (c > path && *c != PATH_SEPARATOR)
//	{
//		c--;
//	}

//	//return (c == path) ? c : c - 1;
//	return (c == path) ? c + 2 : c;
//}

///**
//	Returns the filename of the path.
//@note
//	(e.g. /home/babbeo/texture.tga -> texture.tga).
//@param path
//	The input path
//@param ret
//	The output filename
//@return
//	True if success, false otherwise
//*/
//bool Filesystem::GetFilename(const char* path, char* ret)
//{
//	if (IsRootPath(path.c_str()))
//	{
//		ret = Str("");
//		return true;
//	}

//	RemoveTrailingSeparator(path, ret);

//	if (ret.FindLast(PATH_SEPARATOR) != -1)
//	{
//		ret = path.GetSubstring(path.FindLast(PATH_SEPARATOR) + 1, path.GetLength());
//	}

//	return true;
//}

///**
//	Returns the basename of the path.
//@note
//	(e.g. /home/babbeo/texture.tga -> texture).
//@param path
//	The input path
//@param ret
//	The output basename
//@return
//	True if success, false otherwise
//*/
//bool Filesystem::GetBasename(const char* path, char* ret)
//{
//	if (!GetFilename(path, ret))
//	{
//		return false;
//	}

//	if (ret.FindLast('.') != -1)
//	{
//		ret = ret.GetSubstring(0, ret.FindLast('.'));
//	}

//	return true;
//}

/**
	Returns the extension of the path.
@note
	(e.g. /home/babbeo/texture.tga -> .tga).
@param path
	The input path
@return
	The extension.
*/
const char* Filesystem::GetExtension(const char* relativePath)
{
	assert(relativePath != NULL);

	static char extension[32];

	int32_t i = Str::FindLast(relativePath, '.');

	if (i == -1)
	{
		return Str::EMPTY;
	}

	Str::StrCpy(extension, &relativePath[i]);

	return extension;
}

///**
//	Returns the segments contained in path.
//@param path
//	The input path
//@param ret
//	The output list containing path's segments
//@return
//	True if success, false otherwise
//*/
//bool Filesystem::GetSegments(const char* path, List<Str>& ret)
//{
//	path.Split(PATH_SEPARATOR, ret);

//	if (ret.GetSize() > 0)
//	{
//		return true;
//	}

//	return false;
//}

///**
//	Fills 'ret' with the same path but without the trailing directory separator.
//@note
//	(e.g. /home/babbeo/texture.tga/ -> /home/babbeo/texture.tga).
//@param path
//	The input path
//@param ret
//	The ouput path
//@return
//	True if success, false otherwise
//*/
//bool Filesystem::RemoveTrailingSeparator(const char* path, char* ret)
//{
//	if (path.GetLength() == 0 || IsRootPath(path.c_str()))
//	{
//		ret = path;
//		return true;
//	}

//	if (path[path.GetLength() - 1] == PATH_SEPARATOR)
//	{
//		ret = path.GetSubstring(0, path.GetLength() - 1);
//		return true;
//	}

//	ret = path;
//	return true;
//}

Filesystem filesystem;
Filesystem* GetFilesystem()
{
	return &filesystem;
}

} // namespace crown

