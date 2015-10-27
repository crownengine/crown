/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json_parser.h"
#include "container_types.h"
#include "log.h"
#include "math_utils.h"
#include "network_file.h"
#include "network_filesystem.h"
#include "string_stream.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "types.h"
#include "dynamic_string.h"

namespace crown
{

NetworkFile::NetworkFile(const NetAddress& addr, uint16_t port, const char* filename)
	: File(FOM_READ)
	, _address(addr)
	, _port(port)
	, _position(0)
{
	strncpy(_filename, filename, 1024);
	_socket.connect(addr, port);
}

NetworkFile::~NetworkFile()
{
	_socket.close();
}

void NetworkFile::seek(uint32_t position)
{
	_position = position;
}

void NetworkFile::seek_to_end()
{
	_position = size();
}

void NetworkFile::skip(uint32_t bytes)
{
	_position += bytes;
}

void NetworkFile::read(void* buffer, uint32_t size)
{
	using namespace string_stream;

	TempAllocator1024 alloc;
	StringStream command(alloc);

	// Request the file
	command << "{\"type\":\"filesystem\",\"filesystem\":\"read\",";
	command << "\"file\":\"" << _filename << "\",";
	command << "\"position\":" << _position << ",";
	command << "\"size\":" << size << "}";

	network_filesystem::send(_socket, c_str(command));

	// Wait for response
	Array<char> response(default_allocator());
	network_filesystem::read_response(_socket, response);

	// Parse the response
	JSONParser json(array::begin(response));
	JSONElement root = json.root();

	// DynamicString data_base64;
	// root.key("data").to_string(data_base64);

	// uint32_t out_len = 0;
	// unsigned char* data = base64_decode(data_base64.c_str(), data_base64.length(), &out_len);
	// memcpy(buffer, data, sizeof(unsigned char) * out_len);
	// default_allocator().deallocate(data);
}

void NetworkFile::write(const void* /*buffer*/, uint32_t /*size*/)
{
	CE_FATAL("Cannot write to a network file");
}

bool NetworkFile::copy_to(File& file, uint32_t size)
{
	return true;
}

bool NetworkFile::end_of_file()
{
	return position() == size();
}

bool NetworkFile::is_valid()
{
	return true;
}

void NetworkFile::flush()
{
	// Do nothing
}

uint32_t NetworkFile::position()
{
	return _position;
}

uint32_t NetworkFile::size()
{
	using namespace string_stream;

	TempAllocator1024 alloc;
	StringStream command(alloc);

	// Request the file
	command << "{\"type\":\"filesystem\",\"filesystem\":\"size\",";
	command << "\"file\":\"" << _filename << "\"}";

	network_filesystem::send(_socket, c_str(command));

	// Wait for response
	Array<char> response(default_allocator());
	network_filesystem::read_response(_socket, response);

	JSONParser parser(array::begin(response));
	JSONElement root = parser.root();

	return (uint32_t) root.key("size").to_int();
}

bool NetworkFile::can_read() const
{
	return true;
}

bool NetworkFile::can_write() const
{
	return false;
}

bool NetworkFile::can_seek() const
{
	return true;
}

} // namespace crown

