/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json_parser.h"
#include "network_file.h"
#include "network_filesystem.h"
#include "os.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "string_stream.h"

namespace crown
{

NetworkFilesystem::NetworkFilesystem()
{
}

NetworkFilesystem::NetworkFilesystem(const NetAddress& addr, uint16_t port)
	: _address(addr)
	, _port(port)
{
}

File* NetworkFilesystem::open(const char* path, FileOpenMode mode)
{
	return CE_NEW(default_allocator(), NetworkFile)(_address, _port, path);
}

void NetworkFilesystem::close(File* file)
{
	CE_ASSERT_NOT_NULL(file);
	CE_DELETE(default_allocator(), file);
}

bool NetworkFilesystem::exists(const char* path)
{
	return false;
}

bool NetworkFilesystem::is_directory(const char* path)
{
  return false;
}

bool NetworkFilesystem::is_file(const char* path)
{
  return false;
}

void NetworkFilesystem::create_directory(const char* /*path*/)
{
}

void NetworkFilesystem::delete_directory(const char* /*path*/)
{
}

void NetworkFilesystem::create_file(const char* /*path*/)
{
}

void NetworkFilesystem::delete_file(const char* /*path*/)
{
}

void NetworkFilesystem::list_files(const char* /*path*/, Vector<DynamicString>& /*files*/)
{
}

void NetworkFilesystem::get_absolute_path(const char* path, DynamicString& os_path)
{

}

TCPSocket NetworkFilesystem::new_connection()
{
	TCPSocket socket;
	socket.connect(_address, _port);
	return socket;
}

} // namespace crown
