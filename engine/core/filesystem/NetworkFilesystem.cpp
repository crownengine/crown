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

#include "JSONParser.h"
#include "NetworkFile.h"
#include "NetworkFilesystem.h"
#include "OS.h"
#include "StringUtils.h"
#include "TempAllocator.h"
#include "StringStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
NetworkFilesystem::NetworkFilesystem()
{
}

//-----------------------------------------------------------------------------
NetworkFilesystem::NetworkFilesystem(const NetAddress& addr, uint16_t port)
	: m_address(addr)
	, m_port(port)
{
}

//-----------------------------------------------------------------------------
File* NetworkFilesystem::open(const char* path, FileOpenMode mode)
{
	return CE_NEW(default_allocator(), NetworkFile)(m_address, m_port, path);
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::close(File* file)
{
	CE_ASSERT_NOT_NULL(file);
	CE_DELETE(default_allocator(), file);
}

//-----------------------------------------------------------------------------
bool NetworkFilesystem::is_directory(const char* path)
{

}

//-----------------------------------------------------------------------------
bool NetworkFilesystem::is_file(const char* path)
{

}

//-----------------------------------------------------------------------------
void NetworkFilesystem::create_directory(const char* /*path*/)
{
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::delete_directory(const char* /*path*/)
{
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::create_file(const char* /*path*/)
{
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::delete_file(const char* /*path*/)
{
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::list_files(const char* /*path*/, Vector<DynamicString>& /*files*/)
{
}

//-----------------------------------------------------------------------------
void NetworkFilesystem::get_absolute_path(const char* path, DynamicString& os_path)
{

}

//-----------------------------------------------------------------------------
TCPSocket NetworkFilesystem::new_connection()
{
	TCPSocket socket;
	socket.open(m_address, m_port);
	return socket;
}

} // namespace crown
