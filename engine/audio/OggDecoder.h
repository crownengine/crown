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

#include <vorbis/vorbisfile.h>
#include <cstring>

#include "Types.h"
#include "Assert.h"

#define SOUND_STREAM_BUFFER_SIZE (4096 * 2) // 8K... should be tested

namespace crown
{

//-----------------------------------------------------------------------------
struct OggBuffer
{
	char*	cur_ptr;
	char*	buffer_ptr;
	size_t	length;

	OggBuffer(char* data, const size_t len) : 
		cur_ptr(data), 
		buffer_ptr(data), 
		length(len)
	{
	}
};

//-----------------------------------------------------------------------------
size_t ogg_buffer_read(void* dst, size_t size1, size_t size2, void* src)
{
    OggBuffer* ob = reinterpret_cast<OggBuffer*>(src);

    size_t len = size1 * size2;

    if ((ob->cur_ptr + len) > (ob->buffer_ptr + ob->length))
    {
    	len = ob->buffer_ptr + ob->length - ob->cur_ptr;
    }

    memcpy(dst, ob->cur_ptr, len);

    ob->cur_ptr += len;

    return len;
}

//-----------------------------------------------------------------------------
int32_t	ogg_buffer_seek(void* src, ogg_int64_t offset, int32_t whence)
{
    OggBuffer* ob = reinterpret_cast<OggBuffer*>(src);

    switch(whence)
    {
    	case SEEK_CUR:
    	{
    		ob->cur_ptr += offset;
    		break;
    	}
    	case SEEK_END:
    	{
    		ob->cur_ptr = ob->buffer_ptr + ob->length - offset;
    		break;
    	}
    	case SEEK_SET:
    	{
    		ob->cur_ptr = ob->buffer_ptr + offset;
    		break;
    	}
    	default:
    	{
    		return -1;
    	}
    }

    // Checks inferior limit
    if (ob->cur_ptr < ob->buffer_ptr)
    {
    	ob->cur_ptr = ob->buffer_ptr;
    	return -1;
    }

    // Checks superior limit
    if (ob->cur_ptr > ob->buffer_ptr + ob->length)
    {
    	ob->cur_ptr = ob->buffer_ptr + ob->length;
    	return -1;
    }

    // Success
    return 0;
}

//-----------------------------------------------------------------------------
int32_t ogg_buffer_close(void* src)
{
	(void)src;
	return 0;
}

//-----------------------------------------------------------------------------
long int ogg_buffer_tell(void* src)
{
    OggBuffer* ob = reinterpret_cast<OggBuffer*>(src);

    return ob->cur_ptr - ob->buffer_ptr;
}

//-----------------------------------------------------------------------------
static const char* ov_error_to_string(int32_t error)
{
	Log::i("error: %d", error);
	switch (error)
	{
	case OV_FALSE: return "OV_FALSE";
	case OV_HOLE: return "OV_HOLE";
	case OV_EREAD: return "OV_EREAD";
	case OV_EFAULT: return "OV_EFAULT";
	case OV_EIMPL: return "OV_EIMPL";
	case OV_EINVAL: return "OV_EINVAL";
	case OV_ENOTVORBIS: return "OV_ENOTVORBIS";
	case OV_EBADHEADER: return "OV_EBADHEADER";
	case OV_EVERSION: return "OV_EVERSION";
	case OV_EBADLINK: return "OV_EBADLINK";
	case OV_ENOSEEK: return "OV_ENOSEEK";
	default: return "OV_UNKNOWN"; // this case is never reached
	}
}

//-----------------------------------------------------------------------------
void check_ov_error(int32_t result)
{
	CE_ASSERT(result >= 0, "OV Error: %s", ov_error_to_string(result));
}


//-----------------------------------------------------------------------------
class OggDecoder
{
public:

	//-----------------------------------------------------------------------------
	void init(char* data, const size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		ov_callbacks callbacks;
		callbacks.read_func = ogg_buffer_read;
		callbacks.seek_func = ogg_buffer_seek;
		callbacks.close_func = ogg_buffer_close;
		callbacks.tell_func = ogg_buffer_tell;

		m_buffer = CE_NEW(default_allocator(), OggBuffer)(data, size);

		int32_t result = ov_open_callbacks((void*)m_buffer, &m_stream, NULL, 0, callbacks);
		check_ov_error(result);
		m_info = ov_info(&m_stream, -1);
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		ov_clear(&m_stream);
	}

	//-----------------------------------------------------------------------------
	bool stream()
	{
		int32_t  section;
		int32_t  result;
		int32_t  size = 0;

		while (size < SOUND_STREAM_BUFFER_SIZE)
		{
			result = ov_read(&m_stream, (char*)m_data + size, SOUND_STREAM_BUFFER_SIZE - size, 0, 2, 1, &section);
			check_ov_error(result);

			if (result > 0)
			{
				size += result;
			}
			else if (result == 0)
			{
				return false;
			}
		}

		m_size = size;

		return true;
	}

	//-----------------------------------------------------------------------------
	void rewind()
	{
		int32_t result = ov_raw_seek(&m_stream, 0);
		check_ov_error(result);
	}

	//-----------------------------------------------------------------------------
	const char* data()
	{
		return m_data;
	}

	//-----------------------------------------------------------------------------
	size_t size()
	{
		return m_size;
	}

private:

	OggBuffer* 				m_buffer;

	OggVorbis_File  		m_stream;
	vorbis_info*			m_info;

	char					m_data[SOUND_STREAM_BUFFER_SIZE];
	size_t					m_size;
};

} // namespace crown