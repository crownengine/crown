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

#include "OggDecoder.h"

namespace crown
{

//-----------------------------------------------------------------------------
typedef map<int, OggStream*> StreamMap;

//-----------------------------------------------------------------------------
OggDecoder::OggDecoder()
{
}

//-----------------------------------------------------------------------------
OggDecoder::~OggDecoder()
{
}

//-----------------------------------------------------------------------------
bool OggDecoder::read_page(istream& file, ogg_sync_state* state, ogg_page* page)
{
	int32_t ret = 0;

  // If we've hit end of file we still need to continue processing
  // any remaining pages that we've got buffered.
	if (!file.good())
	{
		return ogg_sync_pageout(state, page) == 1;
	}

	int i = 0;

	while((ret = ogg_sync_pageout(state, page)) != 1) 
	{
		// Returns a buffer that can be written too
		// with the given size. This buffer is stored
		// in the ogg synchronisation structure.
		char* buffer = ogg_sync_buffer(state, 4096);
		assert(buffer);

		// Read from the file into the buffer
		file.read(buffer, 4096);

		int bytes = file.gcount();

		if (bytes == 0) 
		{
		  // End of file. 
		  return false;
    	}

    	// Update the synchronisation layer with the number
    	// of bytes written to the buffer
    	ret = ogg_sync_wrote(state, bytes);
    	assert(ret == 0/*, "Unable to update OGG sync layer"*/);
  	}

  	return true;
}

//-----------------------------------------------------------------------------
void OggDecoder::play(istream& file)
{
	ogg_sync_state state;
	ogg_page page;

	int ret = ogg_sync_init(&state);
	assert(ret == 0/*, "Unable to OGG state"*/);
  
	while (read_page(file, &state, &page))
	{
		int serial = ogg_page_serialno(&page);
		OggStream* stream;

		// If we are at the beginning of logical stream
		if(ogg_page_bos(&page))
		{
			// Read headers and Initialize the stream, giving it the serial
			// number of the stream for this page.
			stream = new OggStream(serial);
			ret = ogg_stream_init(&stream->m_state, serial);
			assert(ret == 0/*, "Unable to init stream"*/);
			m_streams[serial] = stream;
		}

		assert(m_streams.find(serial) != m_streams.end());
		stream = m_streams[serial];

		// Add a complete page to the bitstream
		ret = ogg_stream_pagein(&stream->m_state, &page);
		assert(ret == 0);

		// Return a complete packet of data from the stream
		ogg_packet packet;
		ret = ogg_stream_packetout(&stream->m_state, &packet);
		assert(ret == 0 || ret == 1/*, "Unable to take out a stream packet"*/);
		if (ret == 0) 
		{
			// Need more data to be able to complete the packet
			continue;
		}

	    // A packet is available, this is what we pass to the vorbis or
	    // theora libraries to decode.
	    stream->m_num_packet++;
  }

  // Cleanup
  ret = ogg_sync_clear(&state);
  assert(ret == 0);
}

} // namespace crown

using namespace crown;

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{

	ifstream file(argv[1], ios::in | ios::binary);

	if (file)
	{
		OggDecoder decoder;
		decoder.play(file);
		file.close();

		for (StreamMap::iterator it = decoder.m_streams.begin(); it != decoder.m_streams.end(); ++it)
		{
			OggStream* stream = (*it).second;
      		printf("stream %.4X has %d packets", stream->m_serial, stream->m_num_packet);
      		delete stream;
		}
	}
	return 0;
}