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

#include "Filesystem.h"
#include "Log.h"
#include "OS.h"
#include "DiskFile.h"
#include "Memory.h"
#include "Hash.h"
#include "DiskMountPoint.h"


namespace crown
{

//-----------------------------------------------------------------------------
Filesystem::Filesystem() :
	m_mount_point_head(NULL)
{

}

//-----------------------------------------------------------------------------
Filesystem::~Filesystem()
{
}

//-----------------------------------------------------------------------------
void Filesystem::mount(MountPoint& mp)
{
	if (m_mount_point_head != NULL)
	{
		mp.m_next = m_mount_point_head; 
	}

	m_mount_point_head = &mp;
}

//-----------------------------------------------------------------------------
void Filesystem::umount(MountPoint& mp)
{
	MountPoint* current = m_mount_point_head;
	MountPoint* previous;
	MountPoint* tmp;
	(void)tmp;

	if (&mp == current)
	{	
		tmp = current;

		current = current->m_next;

		tmp = NULL;

		return;
	}
	else
	{
		previous = current;
		current = current->m_next;

		while (current != NULL && &mp != current)
		{
			previous = current;

			current = current->m_next;
		}

		if (current != NULL)
		{
			tmp = current;

			previous->m_next = current->m_next;

			tmp = NULL;

			return;
		}
	}
}

//-----------------------------------------------------------------------------
File* Filesystem::open(const char* mount_point, const char* relative_path, FileOpenMode mode)
{
	MountPoint* mp = find_mount_point(mount_point);

	if (mp)
	{
		return mp->open(relative_path, mode);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void Filesystem::close(File* file)
{
	CE_DELETE(m_allocator, file);
}

//-----------------------------------------------------------------------------
bool Filesystem::exists(const char* mount_point, const char* relative_path)
{
	MountPoint* mp = find_mount_point(mount_point);

	if (mp)
	{
		return mp->exists(relative_path);
	}

	return false;
}

//-----------------------------------------------------------------------------
const char* Filesystem::os_path(const char* mount_point, const char* relative_path)
{
	MountPoint* mp = find_mount_point(mount_point);

	if (mp)
	{
		return ((DiskMountPoint*)mp)->os_path(relative_path);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
MountPoint*	Filesystem::find_mount_point(const char* mount_point)
{
	MountPoint* curr = m_mount_point_head;

	uint32_t type_hash = hash::murmur2_32(mount_point, string::strlen(mount_point), 0);

	while(curr != NULL)
	{
		if (curr->type() == type_hash)
		{
			Log::d("Android mount point found!");
			return curr;
		}

		curr = curr->m_next;
	}

	return NULL;
}


} // namespace crown

