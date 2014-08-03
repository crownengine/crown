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

#include "filesystem.h"
#include "socket.h"
#include "os.h" // for max_path_length
#include "log.h"

namespace crown
{
namespace network_filesystem
{
	inline void read_response(TCPSocket socket, Array<char>& response)
	{
		// Read message length
		uint32_t msg_len = 0;
		socket.read(&msg_len, 4);

		array::resize(response, msg_len);
		socket.read(array::begin(response), msg_len);

		// Ensure NUL-terminated
		array::push_back(response, '\0');
	}

	inline void send(TCPSocket socket, const char* msg)
	{
		uint32_t msg_len = string::strlen(msg);
		socket.write(&msg_len, 4);
		socket.write(msg, msg_len);
	}
} // namespace network_filesystem

/// Access files on a remote file server.
/// All the file paths can be either relative or absolute.
/// When a relative path is given, it is automatically translated
/// to its absolute counterpart based on the file server's root path.
/// Accessing files using absolute path directly is also possible,
/// but platform-specific and thus generally not recommended.
///
/// @ingroup Filesystem
class NetworkFilesystem : public Filesystem
{
public:

	/// Sets the root path to the current working directory of
	/// the engine executable.
	NetworkFilesystem();

	/// Sets the file server address and port.
	/// @note
	/// The @a root_path must be absolute.
	NetworkFilesystem(const NetAddress& addr, uint16_t port);

	/// Opens the file at the given @a path with the given @a mode.
	File* open(const char* path, FileOpenMode mode);

	/// Closes the given @a file.
	void close(File* file);

	/// Returns true if @a path is a directory.
	bool is_directory(const char* path);

	/// Returns true if @a path is a regular file.
	bool is_file(const char* path);

	/// Creates the directory at the given @a path.
	void create_directory(const char* path);

	/// Deletes the directory at the given @a path.
	void delete_directory(const char* path);

	/// Creates the file at the given @a path.
	void create_file(const char* path);

	/// Deletes the file at the given @a path.
	void delete_file(const char* path);

	/// Returns the relative file names in the given @a path.
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the absolute path of the given @a path based on
	/// the root path of the file source. If @a path is absolute,
	/// the given path is returned.
	void get_absolute_path(const char* path, DynamicString& os_path);

private:

	// Returns a new connection to the file server
	TCPSocket new_connection();

private:

	NetAddress m_address;
	uint16_t m_port;
};

} // namespace crown
