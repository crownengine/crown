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

#include "String.h"
#include "OS.h"
#include "Stream.h"

namespace crown
{

struct FilesystemEntry
{
	enum Type
	{
		DIRECTORY = 0,		///< The entry is a directory
		FILE,				///< The entry is a file
		MEMORY,				///< The entry is a memory file (i.e. does not exist on the disk)
		UNKNOWN				///< The entry type is unknown
	};

	FilesystemEntry() : type(UNKNOWN) {}

	Type			type;								///< Type of the entry
	char			os_path[os::MAX_PATH_LENGTH];		///< OS-specific path (use only for debug)
	char			relative_path[os::MAX_PATH_LENGTH];	///< Relative path of the entry
};

class FileStream;

/// Provides a platform-independent way to access files and directories
/// on the host filesystem.
///
/// Accessing files:
/// Every file and every directory must be accessed through the Filesystem.
/// Not a single C/C++ std file io call or other similar facilities
/// should be used in any other part of the engine in order to maintain
/// absolute platform independence.
///
/// Filesystem maintains a root path which acts as base directory for every
/// file operation; access to files outside the root path is not allowed. If
/// you really need it, instantiate another filesystem whith the appropriate
/// root path (e.g.)
///
/// Filesystem fs("/home/foo"); // fs will use "/home/foo" as root path
///
/// fs.is_file("bar.txt");      // file "bar.txt" is relative to the root path,
///                             // so it refers to "/home/foo/bar.txt"
///
/// The filesystem will take care of the necessary path conversions.
/// The root path must be an absolute path for the underlying operating system.
/// Examples of valid root paths:
///
/// 1) "/home/foo"
/// 2) "C:\Users\Phil"
///
/// The relative paths, used to access files, must follow some strict rules:
///
/// a) Only unix-like pathnames (i.e. case sensitive and using '/' as separator)
///    are allowed.
/// b) Only relative paths are allowed: the filesystem is responsible for
///    the creation of its absolute platform-specific counterpart.
/// c) Filesystem forbids pathnames containing '.' and '..' to prevent access to
///    files outside the filesystem's root path.
/// d) Platform specific characters like '/', '\\' and ':' are forbidden as well.
/// e) Symlinks, on platforms which support them, are _not_ resolved for the same
///    reason of c)
/// f) Although mixed-case pathnames are allowed, it is generally safer to use
///    only lower-case ones for maximum compatibility.
///
/// Examples of valid relative paths.
///
/// 1) data/textures/grass.texture
/// 2) grass.texture
/// 3) foo/bar
class Filesystem
{
public:

	/// The @root_path must be absolute.
						Filesystem(const char* root_path);
						~Filesystem();

	/// Returns the root path of the filesystem
	const char*			root_path() const;

	/// Returns whether the @relative_path exists and fills @info with
	/// with informations about the given @relative_path path
	bool				get_info(const char* relative_path, FilesystemEntry& info);
	
	/// Returns whether the @relative_path exists on disk
	bool				exists(const char* relative_path);

	/// Returns whether @relative_path is a regular file
	bool				is_file(const char* relative_path);

	/// Returns whether @relative_path is a directory
	bool				is_dir(const char* relative_path);

	/// Creates a regular file named @relative_path
	bool				create_file(const char* relative_path);

	/// Creates a directory named @relative_path
	bool				create_dir(const char* relative_path);

	/// Deletes the regular file @relative_path
	bool				delete_file(const char* relative_path);

	/// Deletes the directory @relative_path
	bool				delete_dir(const char* relative_path);

	/// Opens the file @relative_path with the specified access @mode
	FileStream*			open(const char* relative_path, StreamOpenMode mode);

	/// Closes a previously opened file @stream
	void				close(FileStream* stream);
	
private:

	// Builds the OS-dependent path from base_path and relative_path
	const char*			build_os_path(const char* base_path, const char* relative_path);
	
private:

	char				m_root_path[os::MAX_PATH_LENGTH];

	// Disable copying
						Filesystem(const Filesystem&);
	Filesystem&			operator=(const Filesystem&);

	friend class		Device;
};

} // namespace crown

