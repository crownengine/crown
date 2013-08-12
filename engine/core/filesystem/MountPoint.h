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

#include "OS.h"
#include "File.h"


namespace crown
{

/// Hashed values for supported MountPoint types
const char* const DISK_MOUNT_POINT			= "disk";
const char* const ANDROID_MOUNT_POINT		= "asset";

const uint32_t DISK_TYPE 					= 0x7BCBC5EE;
const uint32_t ANDROID_TYPE					= 0xAAD5F176;

/// Represent single entity in MountPoint
struct MountPointEntry
{
	enum Type
	{
		DIRECTORY = 0,		/// The entry is a directory
		FILE,				/// The entry is a file
		MEMORY,				/// The entry is a memory file (i.e. does not exist on the disk)
		UNKNOWN				/// The entry type is unknown
	};

	MountPointEntry() : type(UNKNOWN) {}

	Type			type;								/// Type of the entry
	char			os_path[MAX_PATH_LENGTH];			/// OS-specific path (use only for debug)
	char			relative_path[MAX_PATH_LENGTH];		/// Relative path of the entry
};


/// Interface which provides a platform-independent way to access files and directories.
/// Each MountPoint are managed by FileSystem.
/// There may be several types of MountPoint:
///
/// - DiskMountPoint 	- provides interaction with HDD, DVD, BlueRay...
/// - ZipMountPoint  	- provides interaction with compressed archives
/// - NetworkMountPoint	- provides interaction with network
///
/// Accessing files:
/// Every file and every directory must be accessed through the Filesystem's MountPoints.
/// Not a single C/C++ std file io call or other similar facilities
/// should be used in any other part of the engine in order to maintain
/// absolute platform independence.
///
/// MountPoint maintains a root path which acts as base directory for every
/// file operation; access to files outside the root path is not allowed. If
/// you really need it, instantiate another filesystem whith the appropriate
/// root path (e.g.)
///
/// The MountPoint will take care of the necessary path conversions.
/// The root path must be an absolute path for the underlying operating system.
class MountPoint
{
public:
	inline				MountPoint(uint32_t type) : m_type(type) {}

	/// Opens a file and returns a specific instance
	virtual File*		open(const char* path, FileOpenMode mode) = 0;

	/// Close file
	virtual void		close(File* file) = 0;

	/// Returns whether the @a relative_path exists
	virtual bool		exists(const char* relative_path) = 0;

	uint32_t			type() const { return m_type; }

protected:

	MountPoint*			m_next;

	uint32_t			m_type;

	friend class Filesystem;
};

} // namespace crown