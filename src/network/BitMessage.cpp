#include <cassert>
#include <cstring>

#include "BitMessage.h"

namespace crown
{
namespace network
{
  
BitMessage::BitMessage(Allocator& allocator) :
	m_allocator(&allocator),
	m_header(NULL),
	m_data(NULL),
	m_write(NULL), 
	m_read(NULL), 
	m_max_size(0), 
	m_cur_size(0), 
	m_read_count(0),
	m_write_bit(0), 
	m_read_bit(0),
	m_overflowed(false),
	m_init(false)
{
}

//---------------------------------------------------------------------------------------------

BitMessage::~BitMessage()
{
	if (m_header)
	{
		m_allocator->deallocate((void*)m_header);
	}
	if (m_data)
	{
		  m_allocator->deallocate((void*)m_data);
	}
}
//---------------------------------------------------------------------------------------------

uint8_t* BitMessage::get_byte_space(int32_t len)
{
	uint8_t *ptr;

	if (!m_write) 
	{
		printf("cannot write to message");
	}

	// round up to the next byte
	write_byte_align();

	// check for overflow
	check_overflow(len << 3);

	// allocate space
	ptr = m_write + m_cur_size;
	m_cur_size += len;
	
	return ptr;  
}

//---------------------------------------------------------------------------------------------

bool BitMessage::check_overflow(int32_t num_bits)
{
	assert( num_bits >= 0 );
	
	if (num_bits > get_remaining_write_bits()) 
	{
		if (num_bits > (m_max_size << 3)) 
		{
			printf(" %i bits is > full message size", num_bits );
		}
		printf("overflow\n");
		begin_writing();
		m_overflowed = true;
		return true;
	}
	return false;  
}

//---------------------------------------------------------------------------------------------
void BitMessage::init(int32_t len)
{
	m_header = (uint8_t*)m_allocator->allocate(12);
	
	m_data = (uint8_t*)m_allocator->allocate(len);
	
	m_write = m_data;
	m_read = m_data;
	m_max_size = len;
	
	m_init = true;
}

//---------------------------------------------------------------------------------------------
void BitMessage::set_header(uint32_t id, uint16_t sequence, uint16_t ack, uint32_t ack_bits)
{
	uint8_t header[12];
	header[0]	= (uint8_t)(id >> 24);
	header[1]	= (uint8_t)(id >> 16);
	header[2]	= (uint8_t)(id >> 8);
	header[3]	= (uint8_t)id;
	header[4]	= (uint8_t)(sequence >> 8);
	header[5]	= (uint8_t)sequence;
	header[6]	= (uint8_t)(ack >> 8);
	header[7]	= (uint8_t)ack;
	header[8]	= (uint8_t)(ack_bits >> 24);
	header[9]	= (uint8_t)(ack_bits >> 16);
	header[10]	= (uint8_t)(ack_bits >> 8);
	header[11]	= (uint8_t)(ack_bits);
	
	memcpy(m_header, header, 12);
}

//---------------------------------------------------------------------------------------------
uint8_t* BitMessage::get_header()
{
	return m_header;
}

//---------------------------------------------------------------------------------------------
const uint8_t* BitMessage::get_header() const
{
	return m_header;
}
//---------------------------------------------------------------------------------------------
uint8_t* BitMessage::get_data()
{
	return m_write;
}

//---------------------------------------------------------------------------------------------
const uint8_t* BitMessage::get_data() const
{
	return m_read;
}

//---------------------------------------------------------------------------------------------
size_t BitMessage::get_max_size() const
{
	return m_max_size;
}

//---------------------------------------------------------------------------------------------
bool BitMessage::is_overflowed()
{
	return m_overflowed;
}

//---------------------------------------------------------------------------------------------
bool BitMessage::is_init()
{
	return m_init;
}

//---------------------------------------------------------------------------------------------
size_t BitMessage::get_header_size() const
{
	return 12;
}

//---------------------------------------------------------------------------------------------
size_t BitMessage::get_size() const
{
	return m_cur_size;
}

//---------------------------------------------------------------------------------------------

void BitMessage::set_size(size_t size)
{
	if (size > m_max_size)
	{
		m_cur_size = m_max_size;
	}
	else
	{
		m_cur_size = size;
	}
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_write_bit() const
{
	return m_write_bit;
}

//---------------------------------------------------------------------------------------------

void BitMessage::set_write_bit(int32_t bit)
{
	m_write_bit = bit & 7;
	if (m_write_bit) 
	{
		m_write[m_cur_size-1] &= (1 << m_write_bit) - 1;
	}
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_num_bits_written() const
{
	return ((m_cur_size << 3) - ((8 - m_write_bit) & 7));  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_remaining_write_bits() const
{
	return (m_max_size << 3) - get_num_bits_written(); 
}

//---------------------------------------------------------------------------------------------

void BitMessage::save_write_state(int32_t& s,int32_t& b) const
{
	s = m_cur_size;
	b = m_write_bit;
}

//---------------------------------------------------------------------------------------------

void BitMessage::restore_write_state(int32_t s,int32_t b)
{
	m_cur_size = s;
	m_write_bit = b & 7;
	
	if (m_write_bit) 
	{
		m_write[m_cur_size-1] &= (1 << m_write_bit) - 1;
	}  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_read_count() const
{
	return m_read_count;
}

//---------------------------------------------------------------------------------------------

void BitMessage::set_read_count(int32_t bytes)
{
	m_read_count = bytes;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_read_bit() const
{
	return m_read_bit;
}

//---------------------------------------------------------------------------------------------

void BitMessage::set_read_bit(int32_t bit)
{
	m_read_bit = bit & 7;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_num_bits_read() const
{
	return ((m_read_count << 3) - ((8 - m_read_bit) & 7));  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_remaining_read_bits() const
{
	return (m_cur_size << 3) - get_num_bits_read();
}

//---------------------------------------------------------------------------------------------

void BitMessage::save_read_state(int32_t& c, int32_t& b) const
{
	c = m_read_count;
	b = m_read_bit;
}

//---------------------------------------------------------------------------------------------

void BitMessage::restore_read_state(int32_t c, int32_t b)
{
	m_read_count = c;
	m_read_bit = b & 7;
}

//---------------------------------------------------------------------------------------------

void BitMessage::begin_writing()
{
	m_cur_size = 0;
	m_write_bit = 0;
	m_overflowed = false;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_remaining_space() const
{
	return m_max_size - m_cur_size;
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_byte_align()
{
	m_write_bit = 0;
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_bits(int32_t value, int32_t num_bits)
{
	int32_t		put;
	int32_t		fraction;

	// check if m_write is void
	if (!m_write) 
	{
		printf( "cannot write to message" );
	}
	// check if the number of bits is valid
	if (num_bits == 0 || num_bits < -31 || num_bits > 32) 
	{
		printf( "bad numBits %i", num_bits);
	}

	// check for value overflows
	// this should be an error really, as it can go unnoticed and cause either bandwidth or corrupted data transmitted
	if (num_bits != 32) 
	{
		if (num_bits > 0) 
		{
			if (value > (1 << num_bits) - 1) 
			{
				printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < 0) 
			{
				printf( "value overflow %d %d", value, num_bits );
			}
		} 
		else 
		{
			int32_t r = 1 << (-1 - num_bits);
			if (value > r - 1) 
			{
				printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < -r) 
			{
				printf( "value overflow %d %d", value, num_bits );
			}
		}
	}

	// Change sign if it's negative
	if (num_bits < 0 ) 
	{
		num_bits = -num_bits;
	}

	// check for msg overflow
	if (check_overflow(num_bits)) 
	{
		return;	
	}

	// write the bits
	while(num_bits) 
	{
		if (m_write_bit == 0) 
		{
			m_write[m_cur_size] = 0;
			m_cur_size++;
		}
		
		put = 8 - m_write_bit;
		if (put > num_bits) 
		{
			put = num_bits;
		}
		
		fraction = value & ((1 << put) - 1);
		m_write[m_cur_size - 1] |= fraction << m_write_bit;
		num_bits -= put;
		value >>= put;
		m_write_bit = (m_write_bit + put) & 7;
	}
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_int8(int32_t c)
{
	write_bits(c, -8);
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_uint8(int32_t c)
{
	write_bits(c, 8);  
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_int16(int32_t c)
{
	write_bits(c, -16);  
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_uint16(int32_t c)
{
	write_bits(c, 16);
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_int32(int32_t c)
{
	write_bits(c, 32);
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_real(real f)
{
	write_bits(*reinterpret_cast<int32_t *>(&f), 32);  
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_vec3(const Vec3& v)
{
	write_real(v.x);
	write_real(v.y);
	write_real(v.z);
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_string(const char* s, int32_t max_len, bool make_7_bit)
{
	if (!s) 
	{
		write_data("", 1);
	}
	else 
	{
		int32_t i;
		int32_t len = std::strlen(s);
		uint8_t* data_ptr;
		const uint8_t* byte_ptr;
		
		// calculates length
		len = std::strlen(s);
		
		if (max_len >= 0 && len >= max_len) 
		{
			len = max_len - 1;
		}
		
		data_ptr = get_byte_space(len + 1);
		byte_ptr = reinterpret_cast<const uint8_t*>(s);
		if (make_7_bit) 
		{
			for (i = 0; i < len; i++) 
			{
				if ( byte_ptr[i] > 127 ) 
				{
					data_ptr[i] = '.';
				} 
				else 
				{
					data_ptr[i] = byte_ptr[i];
				}
			}
		}
		else 
		{
			for (i = 0; i < len; i++) 
			{
				data_ptr[i] = byte_ptr[i];
			}
		}
		
		data_ptr[i] = '\0';
	}  
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_data(const void* data, int32_t length)
{
	memcpy(get_byte_space(length), data, length);
}

//---------------------------------------------------------------------------------------------

void BitMessage::write_netaddr(const os::NetAddress addr)
{
	uint8_t* ptr;
	
	ptr = get_byte_space(4);
	memcpy(ptr, addr.address, 4);
	write_uint16(addr.port);
}

//---------------------------------------------------------------------------------------------

void BitMessage::begin_reading() const
{
	m_read_count = 0;
	m_read_bit = 0;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::get_remaing_data() const
{
	m_cur_size - m_read_count;
}

//---------------------------------------------------------------------------------------------

void BitMessage::read_byte_align() const
{
	m_read_bit = 0;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_bits(int32_t num_bits) const
{
	int32_t		value;
	int32_t		value_bits;
	int32_t		get;
	int32_t		fraction;
	bool		sgn;

	if (!m_read) 
	{
		printf("cannot read from message");
	}

	// check if the number of bits is valid
	if ( num_bits == 0 || num_bits < -31 || num_bits > 32 ) 
	{
		printf("bad number of bits %i", num_bits );
	}

	value = 0;
	value_bits = 0;

	// change sign if it is negative
	if (num_bits < 0) 
	{
		num_bits = -num_bits;
		sgn = true;
	}
	else 
	{
		sgn = false;
	}

	// check for overflow
	if (num_bits > get_remaining_read_bits()) 
	{
		return -1;
	}

	while (value_bits < num_bits) 
	{
		if (m_read_bit == 0) 
		{
			m_read_count++;
		}
		
		get = 8 - m_read_bit;
		
		if (get > (num_bits - value_bits)) 
		{
			get = num_bits - value_bits;
		}
		
		fraction = m_read[m_read_count - 1];
		fraction >>= m_read_bit;
		fraction &= (1 << get) - 1;
		value |= fraction << value_bits;

		value_bits += get;
		m_read_bit = (m_read_bit + get) & 7;
	}

	if (sgn) 
	{
		if (value & (1 << (num_bits - 1))) 
		{
			value |= -1 ^ (( 1 << num_bits) - 1);
		}
	}

	return value;  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_int8() const
{
	return (int32_t)read_bits(-8);
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_uint8() const
{
  	return (int32_t)read_bits(8);

}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_int16() const
{
	return (int32_t)read_bits(-16);  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_uint16() const
{
	return (int32_t)read_bits(16);  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_int32() const
{
	return (int32_t)read_bits(32);
}

//---------------------------------------------------------------------------------------------

real BitMessage::read_real() const
{
	float value;
	*reinterpret_cast<int*>(&value) = read_bits(32);
	return value;  
}

//---------------------------------------------------------------------------------------------

Vec3 BitMessage::read_vec3() const
{
	Vec3 v;
	
	v.x = read_real();
	v.y = read_real();
	v.z = read_real();
	
	return v;
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_string(char* buffer, int32_t buffer_size) const
{
	int	l = 0;
	int c;

	read_byte_align();
	
	while(1) 
	{
		c = read_uint8();
		if (c <= 0 || c >= 255) 
		{
			break;
		}
		// translate all fmt spec to avoid crash bugs in string routines
		if ( c == '%' ) 
		{
			c = '.';
		}

		// we will read past any excessively long string, so
		// the following data can be read, but the string will
		// be truncated
		if (l < buffer_size - 1) 
		{
			buffer[l] = c;
			l++;
		}
	}
	
	buffer[l] = 0;
	return l;  
}

//---------------------------------------------------------------------------------------------

int32_t BitMessage::read_data(void* data, int32_t length) const
{
	int count;

	read_byte_align();
	
	count = m_read_count;

	if (m_read_count + length > m_cur_size) 
	{
		if (data) 
		{
			memcpy(data, m_read + m_read_count, get_remaing_data());
		}
		m_read_count = m_cur_size;
	} 
	else 
	{
		if (data) 
		{
			memcpy(data, m_read + m_read_count, length);
		}
		m_read_count += length;
	}

	return (m_read_count - count);  
}

//---------------------------------------------------------------------------------------------
void BitMessage::read_netaddr(os::NetAddress* addr) const
{

	for (int i = 0; i < 4; i++) 
	{
		addr->address[i] = read_uint8();
	}
	
	addr->port = read_uint16();  
}

//---------------------------------------------------------------------------------------------
void BitMessage::print() const
{
	os::printf("MAX_SIZE: %d\n", m_max_size);
	os::printf("CUR_SIZE: %d\n", m_cur_size);
}
  
}	//namespace network
}	//namespace crown
