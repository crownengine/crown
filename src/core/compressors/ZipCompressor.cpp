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
//	uint8_t* out_data = (uint8_t*)m_allocator->allocate(sizeof(uint8_t) * in_size);
//	uint8_t* last_byte = out_data;

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
//		read_data_block(in, this_step_bytes); 

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
//				memcpy(last_byte, out, sizeof(uint8_t) * have);
				m_data_list.push(out, have);			
//				last_byte += sizeof(uint8_t) * have;	
				//stream->write_data_block(out, have);
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

}

}
