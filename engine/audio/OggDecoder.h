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

#define SOUND_STREAM_BUFFER_SIZE (4096 * 8) // 32K... should be tested

namespace crown
{

//-----------------------------------------------------------------------------
struct OggBuffer
{
	char*	cur_ptr;
	char*	buffer_ptr;
	size_t	length;

	OggBuffer(char* data, size_t len) : cur_ptr(data), buffer_ptr(data), length(len) {}
};

//-----------------------------------------------------------------------------
size_t ogg_buffer_read(void* dst, size_t size, size_t num, void* src)
{
    OggBuffer* ob = reinterpret_cast<OggBuffer*>(src);

    size_t len = size * num;

    if ((ob->cur_ptr + len) > (ob->buffer_ptr + ob->length))
    {
    	len = ob->buffer_ptr + ob->length - ob->cur_ptr;
    }

    memcpy(dst, ob->cur_ptr, len);

    ob->cur_ptr += len;

    return len;
}

//-----------------------------------------------------------------------------
int32_t	ogg_buffer_seek(void * src, ogg_int64_t offset, int32_t whence)
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
//-----------------------------------------------------------------------------

/// __OggDecorer__ decodes ogg buffers and provides peaces of decoded audio data.
class OggDecorer
{
public:

	//-----------------------------------------------------------------------------
	OggDecorer() {}

	//-----------------------------------------------------------------------------
	void open(const uint8_t* buffer)
	{
		ov_callbacks callbacks;
		callbacks.read_func = ogg_buffer_read;
		callbacks.seek_func = ogg_buffer_seek;
		callbacks.close_func = ogg_buffer_close;
		callbacks.tell_func = ogg_buffer_tell;

		int32_t result = ov_open_callbacks(buffer, m_stream, NULL, 0, callbacks);
		CE_ASSERT(result == 0, "Unable to open stream buffer");

		m_info = ov_info(&m_stream, -1);

		m_comment = ov_comment(&m_stream, -1);
	}

	//-----------------------------------------------------------------------------
	void stream()
	{
	    int32_t  size = 0;
	    int32_t  section;
	    int32_t  result;

	    while (size < SOUND_STREAM_BUFFER_SIZE)
	    {
	    	result = ov_read(&m_stream, m_data + size, SOUND_STREAM_BUFFER_SIZE - size, 0, 2, 1, &section);

	    	CE_ASSERT(result >= 0, "Fail to read and decode ogg stream");

	    	if (result > 0)
	    	{
	    		size += result;
	    	}
	    	else
	    	{
	    		break;
	    	}
	    }	
	}
	
	//-----------------------------------------------------------------------------
	void close()
	{
		ov_clear(m_stream);
		memset(m_data, 0, SOUND_STREAM_BUFFER_SIZE);
		m_info = NULL;
	}

	//-----------------------------------------------------------------------------
	const uint8_t* data()
	{
		return m_data;
	}

private:

	OggVorbis_File  		m_stream;
	vorbis_info*			m_info;
	vorbis_comment*			m_comment;

	uint8_t					m_data[SOUND_STREAM_BUFFER_SIZE];
};




} // namespace crown