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

#include "Str.h"
#include "List.h"
#include "Stream.h"
#include "Log.h"

namespace Crown
{

/**
	Enumerates the filesystem's entry types
*/
enum FilesystemEntryType
{
	FET_DIR = 0,		//!< The entry is a directory
	FET_FILE,			//!< The entry is a file
	FET_MEM,			//!< The entry is a memory file (i.e. does not exist on the disk)
	FET_UNKNOWN			//!< The entry type is unknown
};

struct FilesystemEntry
{
	FilesystemEntry() : type(FET_UNKNOWN)
	{
	}

	void PrintReport()
	{
		Log::I("OSPath\t\t: %s", osPath.c_str());
		Log::I("RelPath\t\t: %s", relativePath.c_str());
		Log::I("Type:\t\t: %s", ((type == FET_FILE) ? "file" : (type == FET_DIR) ? "dir" : "unknown"));
	}

	FilesystemEntryType	type;			//!< Type of the entry
	Str					osPath;			//!< OS-specific path (use only for debug)
	Str					relativePath;	//!< Relative path of the entry
};

class Stream;

/**
	Virtual filesystem.

	Provides a platform-independent way to access files and directories
	on the host filesystem.

	Accessing files:
	Every file and every directory is accessed through the filesystem.
	Not a single C/C++ std file io call should be used in any other part
	of the engine in order to maintain platform independence.

	Pathnames:
	Only unix-like pathnames (i.e. case sensitive and using '/' as separator)
	are allowed.
	Only relative paths are allowed: the filesystem is responsible for
	the creation of its absolute platform-specific counterpart.

	Filesystem forbids pathnames containing '.' and '..' to prevent access
	to files outside the predefined directories.
	Platform specific characters like '/', '\\' and ':' are forbidden as well.

	Although mixed-case pathnames are allowed, it is generally safer to use
	only lower-case ones for maximum compatibility.

	Filesystem provides access to two main directories:
		1) Root directory
			it is the directory where all of the data came from.
			Defaults to the directory where the executable is in
			but can be overridden by the CROWN_ROOT_PATH environment
			variable. The root directory is set only once at the
			engine start and cannot be changed anymore.
		2) User directory
			defaults to the user home directory. Used to store user-specific
			stuffs such as config files, screenshots, savegames ecc.
*/
class Filesystem
{

public:

						Filesystem();
						~Filesystem();

	void				Init(const char* rootPath, const char* userPath);

	const char*			GetRootPath() const;
	const char*			GetUserPath() const;

	void				SetUserPath(const char* relativePath);

	const char*			BuildOSPath(const char* basePath, const char* relativePath);

	bool				GetInfo(const char* basePath, const char* relativePath, FilesystemEntry& info);
	bool				Exists(const char* relativePath);

	bool				IsFile(const char* relativePath);
	bool				IsDir(const char* relativePath);

	bool				CreateFile(const char* relativePath);
	bool				CreateDir(const char* relativePath);

	bool				DeleteFile(const char* relativePath);
	bool				DeleteDir(const char* relativePath);

	Stream*				OpenStream(const char* relativePath, StreamOpenMode openMode);
	Stream*				OpenRead(const char* relativePath);
	Stream*				OperWrite(const char* relativePath);
	Stream*				OpenAppend(const char* relativePath);

	void				Close(Stream* stream);

	void				PrintReport();

	static bool			IsValidSegment(const char* segment);
	static bool			IsValidPath(const char* path);
	static bool			IsAbsolutePath(const char* path);
	static bool			IsRootPath(const char* path);

//	static const char*	GetPathname(const char* path);
//	static bool			GetFilename(const char* path, char* ret);
//	static bool			GetBasename(const char* path, char* ret);
	static const char*	GetExtension(const char* relativePath);
//	static bool			GetSegments(const char* path, List<Str>& ret);

//	static bool			RemoveTrailingSeparator(const char* path, char* ret);

private:

	void				_SetRootPath(const char* relativePath) { mRootPath = relativePath; }

	bool				mIsInit;

	Str					mRootPath;			//!< The root path.
	Str					mUserPath;			//!< Where user settings/saves/ecc. live in

	// Disable copying
	Filesystem(const Filesystem&);
	Filesystem& operator=(const Filesystem&);

	friend class		Device;
};

Filesystem* GetFilesystem();

} // namespace Crown

