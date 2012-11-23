#include <cstring>

#include "zlib.h"
#include "MathUtils.h"
#include "ZipCompressor.h"

namespace crown
{

ZipCompressor::ZipCompressor(Allocator& allocator) :
	Compressor(allocator),
	m_data_list(allocator)
{
	
}

ZipCompressor::~ZipCompressor()
{

}

uint8_t* ZipCompressor::compress(void* data, size_t in_size, size_t& out_size)
{
	assert(data != NULL);
	assert(in_size > 0);

	m_data_list.clear();

	// memory's chunk max dimension
	const size_t CHUNK_SIZE = 16384;
	// incoming buffer
	uint8_t in[CHUNK_SIZE];
	// outgoing buffer
	uint8_t out[CHUNK_SIZE];

	int32_t ret;
	int32_t flush;
	unsigned have;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, 6);

	assert(ret == Z_OK);

	size_t bytes_read = 0;

	do
	{
		size_t this_step_bytes = math::min(CHUNK_SIZE, in_size - bytes_read);
		memcpy(in, (uint8_t*)data, sizeof(uint8_t) * this_step_bytes);

		strm.avail_in = this_step_bytes;
		strm.next_in = in;

		flush = (in_size - bytes_read) <= CHUNK_SIZE ? Z_FINISH : Z_NO_FLUSH;

		do
		{
			strm.avail_out = CHUNK_SIZE;
			strm.next_out = out;

			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

			have = CHUNK_SIZE - strm.avail_out;
			if (have > 0)
			{
				m_data_list.push(out, have);			
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		bytes_read += this_step_bytes;
		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);

	out_size = strm.total_out;

	return m_data_list.begin(); 
}

uint8_t* ZipCompressor::decompress(const void* data, size_t in_size, size_t& out_size)
{
	m_data_list.clear();
	
	const size_t CHUNK_SIZE = 16384;
	int32_t ret;
	unsigned have;
	z_stream strm;
	uint8_t in[CHUNK_SIZE];
	uint8_t out[CHUNK_SIZE];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	
	assert(ret == Z_OK);

	size_t bytes_read = 0;

	/* decompress until deflate stream ends or end of file */
	do
	{
		size_t this_step_bytes = math::min(CHUNK_SIZE, in_size - bytes_read);
		memcpy(in, (uint8_t*)data, sizeof(uint8_t) * this_step_bytes);

		strm.avail_in = this_step_bytes;
		strm.next_in = in;
		if (strm.avail_in == 0)
		{
				break;
		}

		/* run inflate() on input until output buffer not full */
		do {
				strm.avail_out = CHUNK_SIZE;
				strm.next_out = out;
				ret = inflate(&strm, Z_NO_FLUSH);
				
				assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
				
				switch (ret) 
				{
					case Z_NEED_DICT:
					{
							ret = Z_DATA_ERROR;     /* and fall through */
					}
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
					{
							(void)inflateEnd(&strm);
							return false;
					}
				}
				have = CHUNK_SIZE - strm.avail_out;
				if (have > 0)
				{
					m_data_list.push(out, have);			
				}
		} while (strm.avail_out == 0);

		bytes_read += this_step_bytes;
		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	
	out_size = strm.total_out;

	return m_data_list.begin();
}

}
