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

#include <iostream>
#include <fstream>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <map>
#include <cassert>
#include <stdint.h>

using namespace std;

namespace crown
{

//-----------------------------------------------------------------------------
struct OggStream
{
	int					m_serial;
	int					m_num_packet;
	ogg_stream_state 	m_state;

	inline OggStream(int serial = -1) : m_serial(serial), m_num_packet(0) {}

	inline ~OggStream() { int ret = ogg_stream_clear(&m_state);	//CE_ASSERT(ret == 0, "Cannot clear OGG stream");	}
};

//-----------------------------------------------------------------------------
typedef map<int, OggStream*> StreamMap;

//-----------------------------------------------------------------------------
class OggDecoder
{
public:

					OggDecoder();
					~OggDecoder();

	bool 			read_page(istream& file, ogg_sync_state* state, ogg_page* page);

	void 			play(istream& file);

public:
	StreamMap		m_streams;
};

} // namespace crown