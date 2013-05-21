/*
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
namespace os
{

/// OS level network address.
class NetAddress
{
public:

	/// Initializes the address to 127.0.0.1 and port to 1000.
					NetAddress();

	/// Initializes the address from IP address (as single elemets)
	/// and the port number.
					NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);

	
	/// Returns the IP address as packed 32-bit integer.
	uint32_t		address() const;

	/// Returns the port number bind to the address.
	uint16_t		port() const;

	/// Sets both the IP address (packed 32-bit integer) and the port number.
	void			set(uint32_t address, uint16_t port);

	/// Sets both the IP address (as single elements) and the port number.
	void			set(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);
	
	bool			operator==(const NetAddress& addr) const;
	NetAddress&		operator=(const NetAddress& addr);
	
	void			print() const;

public:

	uint8_t 		m_address[4];
	uint16_t 		m_port;
};

//-----------------------------------------------------------------------------
inline NetAddress::NetAddress() :
	m_port(1000)
{
	m_address[0] = 127;
	m_address[1] = 0;
	m_address[2] = 0;
	m_address[3] = 1;
}

//-----------------------------------------------------------------------------
inline NetAddress::NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port) :
	m_port(port)
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
inline uint16_t NetAddress::port() const
{
	return m_port;
}

//-----------------------------------------------------------------------------
inline void NetAddress::set(uint32_t address, uint16_t port)
{
	m_address[0] = address >> 24;
	m_address[1] = address >> 16;
	m_address[2] = address >> 8;
	m_address[3] = address;

	m_port = port;
}

//-----------------------------------------------------------------------------
inline void NetAddress::set(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
{
	m_address[0] = a;
	m_address[1] = b;
	m_address[2] = c;
	m_address[3] = d;
	
	m_port = port;
}

//-----------------------------------------------------------------------------
inline bool NetAddress::operator==(const NetAddress& addr) const
{
	return m_address[0] == addr.m_address[0] &&
		   m_address[1] == addr.m_address[1] &&
		   m_address[2] == addr.m_address[2] &&
		   m_address[3] == addr.m_address[3] &&
		   m_port == addr.m_port;
}

//-----------------------------------------------------------------------------
inline NetAddress& NetAddress::operator=(const NetAddress& addr)
{
	m_address[0] = addr.m_address[0];
	m_address[1] = addr.m_address[1];
	m_address[2] = addr.m_address[2];
	m_address[3] = addr.m_address[3];
	
	m_port = addr.m_port;
	
	return *this;
}

//-----------------------------------------------------------------------------
inline void NetAddress::print() const
{
	os::printf("NetAddress: %i.%i.%i.%i:%i\n", m_address[0], m_address[1], m_address[2], m_address[3], m_port);
}

} // namespace os
} // namespace crown
