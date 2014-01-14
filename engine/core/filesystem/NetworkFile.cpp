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
#include "List.h"
#include "Log.h"
#include "MathUtils.h"
#include "NetworkFile.h"
#include "NetworkFilesystem.h"
#include "StringStream.h"
#include "StringUtils.h"
#include "TempAllocator.h"
#include "Types.h"
#include "DynamicString.h"

namespace crown
{

//-----------------------------------------------------------------------------
NetworkFile::NetworkFile(const NetAddress& addr, uint16_t port, const char* filename)
	: File(FOM_READ)
	, m_address(addr)
	, m_port(port)
	, m_position(0)
{
	string::strncpy(m_filename, filename, MAX_PATH_LENGTH);
	m_socket.connect(addr, port);
}

//-----------------------------------------------------------------------------
NetworkFile::~NetworkFile()
{
	m_socket.close();
}

//-----------------------------------------------------------------------------
void NetworkFile::seek(size_t position)
{
	m_position = position;
}

//-----------------------------------------------------------------------------
void NetworkFile::seek_to_end()
{
	m_position = size();
}

//-----------------------------------------------------------------------------
void NetworkFile::skip(size_t bytes)
{
	m_position += bytes;
}

//-----------------------------------------------------------------------------
void NetworkFile::read(void* buffer, size_t size)
{
	TempAllocator1024 alloc;
	StringStream command(alloc);

	// Request the file
	command << "{\"type\":\"filesystem\",\"filesystem\":\"read\",";
	command << "\"file\":\"" << m_filename << "\",";
	command << "\"position\":" << m_position << ",";
	command << "\"size\":" << size << "}";

	network_filesystem::send(m_socket, command.c_str());

	// Wait for response
	List<char> response(default_allocator());
	network_filesystem::read_response(m_socket, response);

	// Parse the response
	JSONParser json(response.begin());
	JSONElement root = json.root();

	DynamicString data_base64;
	root.key("data").string_value(data_base64);

	size_t out_len = 0;
	unsigned char* data = math::base64_decode(data_base64.c_str(), data_base64.length(), &out_len);
	memcpy(buffer, data, sizeof(unsigned char) * out_len);
	default_allocator().deallocate(data);
}

//-----------------------------------------------------------------------------
void NetworkFile::write(const void* /*buffer*/, size_t /*size*/)
{
	CE_FATAL("Cannot write to a network file");
}

//-----------------------------------------------------------------------------
bool NetworkFile::copy_to(File& file, size_t size)
{
	return true;
}

//-----------------------------------------------------------------------------
bool NetworkFile::end_of_file()
{
	return position() == size();
}

//-----------------------------------------------------------------------------
bool NetworkFile::is_valid()
{
	return true;
}

//-----------------------------------------------------------------------------
void NetworkFile::flush()
{
	// Do nothing
}

//-----------------------------------------------------------------------------
size_t NetworkFile::position()
{
	return m_position;
}

//-----------------------------------------------------------------------------
size_t NetworkFile::size()
{
	TempAllocator1024 alloc;
	StringStream command(alloc);

	// Request the file
	command << "{\"type\":\"filesystem\",\"filesystem\":\"size\",";
	command << "\"file\":\"" << m_filename << "\"}";

	network_filesystem::send(m_socket, command.c_str());

	// Wait for response
	List<char> response(default_allocator());
	network_filesystem::read_response(m_socket, response);

	JSONParser parser(response.begin());
	JSONElement root = parser.root();

	return (size_t) root.key("size").int_value();
}

//-----------------------------------------------------------------------------
bool NetworkFile::can_read() const
{
	return true;
}

//-----------------------------------------------------------------------------
bool NetworkFile::can_write() const
{
	return false;
}

//-----------------------------------------------------------------------------
bool NetworkFile::can_seek() const
{
	return true;
}

} // namespace crown

