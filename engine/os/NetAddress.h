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

#include "Types.h"
#include "OS.h"

namespace crown
{

/// OS level network address.
class NetAddress
{
public:

	/// Initializes the address to 127.0.0.1
					NetAddress();

	/// Initializes the address from IP address (as single elemets)
					NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

	
	/// Returns the IP address as packed 32-bit integer.
	uint32_t		address() const;

	/// Sets both the IP address (packed 32-bit integer)
	void			set(uint32_t address);

	/// Sets both the IP address (as single elements)
	void			set(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

	bool			operator==(const NetAddress& addr) const;
	NetAddress&		operator=(const NetAddress& addr);
	
	void			print() const;

public:

	uint8_t 		m_address[4];
};

//-----------------------------------------------------------------------------
inline NetAddress::NetAddress()
{
	m_address[0] = 127;
	m_address[1] = 0;
	m_address[2] = 0;
	m_address[3] = 1;
}

//-----------------------------------------------------------------------------
inline NetAddress::NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	m_address[0] = a;
	m_address[1] = b;
	m_address[2] = c;
	m_address[3] = d;
}

//-----------------------------------------------------------------------------
inline uint32_t NetAddress::address() const
{
	uint32_t addr = (m_address[0] << 24) | (m_address[1] << 16) | (m_address[2] << 8) | (m_address[3]);

	return addr;
}

//-----------------------------------------------------------------------------
inline void NetAddress::set(uint32_t address)
{
	m_address[0] = address >> 24;
	m_address[1] = address >> 16;
	m_address[2] = address >> 8;
	m_address[3] = address;
}

//-----------------------------------------------------------------------------
inline void NetAddress::set(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	m_address[0] = a;
	m_address[1] = b;
	m_address[2] = c;
	m_address[3] = d;
}

//-----------------------------------------------------------------------------
inline bool NetAddress::operator==(const NetAddress& addr) const
{
	return m_address[0] == addr.m_address[0] &&
		   m_address[1] == addr.m_address[1] &&
		   m_address[2] == addr.m_address[2] &&
		   m_address[3] == addr.m_address[3];
}

//-----------------------------------------------------------------------------
inline NetAddress& NetAddress::operator=(const NetAddress& addr)
{
	m_address[0] = addr.m_address[0];
	m_address[1] = addr.m_address[1];
	m_address[2] = addr.m_address[2];
	m_address[3] = addr.m_address[3];
	
	return *this;
}

//-----------------------------------------------------------------------------
inline void NetAddress::print() const
{
	os::printf("NetAddress: %i.%i.%i.%i\n", m_address[0], m_address[1], m_address[2], m_address[3]);
}

} // namespace crown
