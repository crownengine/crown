#include <iostream>
#include <cassert>

#include "NetCrown.h"
#include "IPv4Address.h"
#include "UDPSocket.h"

namespace crown
{
  
namespace network
{
	UDPSocket::UDPSocket()
	{
		m_socket = 0;
	}

	UDPSocket::~UDPSocket()
	{
		close();
	}

	bool UDPSocket::open(unsigned short port)
	{
		assert(!is_open());

		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_socket <= 0)
		{
			cout << "Failed to create socket." << endl;
			m_socket = 0;
			return false;
		}

		// bind to port

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		if ( bind( m_socket, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			cout << "Failed to bind socket" << endl;
			close();
			return false;
		}

		// set non-blocking io

		#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

		int non_blocking = 1;
		if (fcntl( m_socket, F_SETFL, O_NONBLOCK, non_blocking ) == -1)
		{
			cout << "Failed to set non-blocking socket" << endl;
			close();
			return false;
		}

		#elif PLATFORM == PLATFORM_WINDOWS

		DWORD non_blocking = 1;
		if (ioctlsocket( socket, FIONBIO, &non_blocking ) != 0)
		{
			cout << "Failed to set non-blocking socket" << endl;
			close();
			return false;
		}

		#endif

		return true;
	}

	bool UDPSocket::send(IPv4Address &receiver, const void* data, int size)
	{
		assert(data);
		assert(size > 0);

		if (m_socket == 0)
		{
			return false;
		}
	
		assert(receiver.get_address() != 0);
		assert(receiver.get_port() != 0);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(receiver.get_address());
		address.sin_port = htons(receiver.get_port());

		int sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

		return sent_bytes == size;
	}

	int UDPSocket::receive(IPv4Address &sender, void* data, int size)
	{
		assert(data);
		assert(size > 0);

		if (m_socket == 0)
		{
			return false;
		}

		#if PLATFORM == PLATFORM_WINDOWS
		typedef int socklen_t;
		#endif

		sockaddr_in from;
		socklen_t from_length = sizeof(from);

		int received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

		if (received_bytes <= 0)
		{
				return 0;
		}

		unsigned int address = ntohl(from.sin_addr.s_addr);
		unsigned short port = ntohs(from.sin_port);

		sender = IPv4Address(address, port);

		return received_bytes;
	}

	void UDPSocket::close()
	{
		if ( m_socket != 0 )
		{
			#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
			::close(m_socket);
			#elif PLATFORM == PLATFORM_WINDOWS
			closesocket(m_socket);
			#endif
			m_socket = 0;
		}
	}

	bool UDPSocket::is_open()
	{
		return m_socket != 0;
	}
}
}
