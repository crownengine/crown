#include "PacketQueue.h"

namespace crown
{
namespace network
{

//-------------------------------------------------------------
PacketQueue::PacketQueue() :
	first(0),
	last(0),
	start_index(0),
	end_index(0)
{

}

//-------------------------------------------------------------
PacketQueue::~PacketQueue()
{

}

//-------------------------------------------------------------
bool PacketQueue::add(const PacketData& pd)
{
	
}

//-------------------------------------------------------------
bool PacketQueue::get(PacketData& pd)
{

}

//-------------------------------------------------------------
size_t PacketQueue::get_total_size() const
{

}

//-------------------------------------------------------------
size_t PacketQueue::get_space_left() const
{

}

//-------------------------------------------------------------
int32_t PacketQueue::get_first() const
{

}

//-------------------------------------------------------------
int32_t PacketQueue::get_last() const
{

}

//-------------------------------------------------------------
void PacketQueue::write_bits(int32_t value, int32_t num_bits)
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

//-------------------------------------------------------------
void PacketQueue::write_uint16(int32_t value)
{
	write_bits(c, 16);
}

//-------------------------------------------------------------
void PacketQueue::write_int32(int32_t value)
{
	write_bits(c, 32);
}

//-------------------------------------------------------------
int32_t PacketQueue::read_bits(int32_t num_bits)
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

//-------------------------------------------------------------
int32_t PacketQueue::read_uint16()
{
	return (int32_t)read_bits(16);  
}

//-------------------------------------------------------------
int32_t PacketQueue::read_int32()
{
	return (int32_t)read_bits(32);  
}

}
} // namespace crown
