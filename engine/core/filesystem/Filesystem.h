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

#include "StringUtils.h"
#include "OS.h"
#include "File.h"
#include "HeapAllocator.h"
#include "MountPoint.h"

namespace crown
{

class File;

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

						Filesystem();
						~Filesystem();

	/// Makes available mount point @a mp
	void				mount(MountPoint& mp);

	/// Makes unavailable mount point @a mp
	void				umount(MountPoint& mp);

	/// Opens the file @a relative_path with the specified access @a mode
	/// contained in @a mount_point
	File*				open(const char* mount_point, const char* relative_path, FileOpenMode mode);

	/// Closes a previously opened file @a stream
	void				close(File* stream);

	/// Returns true if file @a relative_path exists in @a mount_point
	bool				exists(const char* mount_point, const char* relative_path);

	/// Returns path of file @a relative_path in @a mount_point
	const char*			os_path(const char* mount_point, const char* relative_path);

private:
	
	/// Returns the first mount point according to @a mount_point or NULL.
	MountPoint*			find_mount_point(const char* mount_point);				

	// Disable copying
						Filesystem(const Filesystem&);
	Filesystem&			operator=(const Filesystem&);
		
private:

	char				m_root_path[MAX_PATH_LENGTH];
	MountPoint* 		m_mount_point_head;

	friend class		Device;
};

} // namespace crown

