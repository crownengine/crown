/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "socket.h"
#include "os.h"
#include "file.h"

namespace crown
{

/// Access file on a remote file server.
///
/// @ingroup Filesystem
class NetworkFile: public File
{
public:

	/// Reads the file named @a filename from the given @a socket.
	NetworkFile(const NetAddress& addr, uint16_t port, const char* filename);
	virtual ~NetworkFile();

	/// @copydoc File::seek()
	void seek(uint32_t position);

	/// @copydoc File::seek_to_end()
	void seek_to_end();

	/// @copydoc File::skip()
	void skip(uint32_t bytes);

	/// @copydoc File::read()
	void read(void* buffer, uint32_t size);

	/// @copydoc File::write()
	void write(const void* buffer, uint32_t size);

	/// @copydoc File::copy_to()
	bool copy_to(File& file, uint32_t size = 0);

	/// @copydoc File::flush()
	void flush();

	/// @copydoc File::end_of_file()
	bool end_of_file();

	/// @copydoc File::is_valid()
	bool is_valid();

	/// @copydoc File::size()
	uint32_t size();

	/// @copydoc File::position()
	uint32_t position();

	/// @copydoc File::can_read()
	bool can_read() const;

	/// @copydoc File::can_write()
	bool can_write() const;

	/// @copydoc File::can_seek()
	bool can_seek() const;

private:

	char _filename[1024];
	NetAddress _address;
	uint16_t _port;
	TCPSocket _socket;
	uint32_t _position;
};

} // namespace crown
