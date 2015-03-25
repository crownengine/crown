/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "os_file.h"
#include "file.h"

namespace crown
{

/// Provides common facilities to access files on disk.
///
/// @ingroup Filesystem
class DiskFile: public File
{
public:

	/// Opens @a filename with specified @a mode
	DiskFile(FileOpenMode mode, const char* filename);
	virtual ~DiskFile();

	/// @copydoc File::seek()
	void seek(size_t position);

	/// @copydoc File::seek_to_end()
	void seek_to_end();

	/// @copydoc File::skip()
	void skip(size_t bytes);

	/// @copydoc File::read()
	void read(void* buffer, size_t size);

	/// @copydoc File::write()
	void write(const void* buffer, size_t size);

	/// @copydoc File::copy_to()
	bool copy_to(File& file, size_t size = 0);

	/// @copydoc File::flush()
	void flush();

	/// @copydoc File::end_of_file()
	bool end_of_file();

	/// @copydoc File::is_valid()
	bool is_valid();

	/// @copydoc File::size()
	size_t size();

	/// @copydoc File::position()
	size_t position();

	/// @copydoc File::can_read()
	bool can_read() const;

	/// @copydoc File::can_write()
	bool can_write() const;

	/// @copydoc File::can_seek()
	bool can_seek() const;

protected:

	OsFile _file;
	bool _last_was_read;

protected:

	inline void check_valid() const
	{
		CE_ASSERT(_file.is_open(), "File is not open");
	}
};

} // namespace crown
