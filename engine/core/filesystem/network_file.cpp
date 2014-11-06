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
	strncpy(_filename, filename, MAX_PATH_LENGTH);
	_socket.connect(addr, port);
}

NetworkFile::~NetworkFile()
{
	_socket.close();
}

void NetworkFile::seek(size_t position)
{
	_position = position;
}

void NetworkFile::seek_to_end()
{
	_position = size();
}

void NetworkFile::skip(size_t bytes)
{
	_position += bytes;
}

void NetworkFile::read(void* buffer, size_t size)
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

	// size_t out_len = 0;
	// unsigned char* data = math::base64_decode(data_base64.c_str(), data_base64.length(), &out_len);
	// memcpy(buffer, data, sizeof(unsigned char) * out_len);
	// default_allocator().deallocate(data);
}

void NetworkFile::write(const void* /*buffer*/, size_t /*size*/)
{
	CE_FATAL("Cannot write to a network file");
}

bool NetworkFile::copy_to(File& file, size_t size)
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

size_t NetworkFile::position()
{
	return _position;
}

size_t NetworkFile::size()
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

	return (size_t) root.key("size").to_int();
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

