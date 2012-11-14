/**
	Internet IPv4 address
	Used for socket connection over TCP/IP or UDP
	Format -> A.B.C.D:port
*/
namespace crown
{
  
namespace network
{
	class IPv4Address
	{
		public:
		
		// Default constructor
		IPv4Address()
		{
			m_address = 0;
			m_port = 0;
		}
		
		// Constructor
		IPv4Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port)
		{
			m_address = (a << 24) | (b << 16) | (c << 8) | d;
			m_port = port;
		}
	
		// Constructor	
		IPv4Address(unsigned int address, unsigned short port)
		{
			m_address = address;
			m_port = port;
		}
	
		inline unsigned int get_address() const
		{
			return m_address;
		}
	
		inline unsigned char get_a() const
		{
			return (unsigned char) (m_address >> 24);
		}
		
		inline unsigned char get_b() const
		{
			return (unsigned char) (m_address >> 16);
		}
		
		inline unsigned char get_c() const
		{
			return (unsigned char) (m_address >> 8);
		}
		
		inline unsigned char get_d() const
		{
			return (unsigned char) (m_address);
		}
		
		inline unsigned short get_port() const
		{ 
			return m_port;
		}
		
		bool operator == (const IPv4Address& other) const
		{
			return m_address == other.m_address && m_port == other.m_port;
		}
		
		bool operator != (const IPv4Address& other) const
		{
			return !(*this == other);
		}
		
		bool operator < (const IPv4Address& other) const
		{
			// note: this is so we can use address as a key in std::map
			if (m_address < other.m_address)
				return true;
			if (m_address > other.m_address)
				return false;
			else
				return m_port < other.m_port;
		}
		
		private:
		
			unsigned int 	m_address;
			unsigned short 	m_port;
	};
}

}

