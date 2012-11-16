#include <cassert>

#include "Message.h"

namespace crown
{
  
namespace network
{
  
Message::Message() : w_data(NULL), r_data(NULL), max_size(0), cur_size(0), write_bit(0), read_count(0), read_bit(0)
{
  
}

Message::~Message()
{
  
}

uchar* Message::get_byte_space(int len)
{
	byte *ptr;

	if (!w_data) 
	{
		os::printf( "idBitMsg::GetByteSpace: cannot write to message" );
	}

	// round up to the next byte
	write_byte_align();

	// check for overflow
	check_overflow(len << 3);

	// allocate space
	ptr = w_data + cur_size;
	cur_size += len;
	
	return ptr;  
}

bool Message::check_overflow(int num_bits)
{
	assert( num_bits >= 0 );
	
	if (num_bits > get_remaining_write_bits()) 
	{
		if (num_bits > (max_size << 3)) 
		{
			os::printf(" %i bits is > full message size", num_bits );
		}
		os::printf("overflow\n");
		begin_writing();
		overflowed = true;
		return true;
	}
	return false;  
}

void Message::init(uchar *data, int len)
{
	w_data = data;
	r_data = data;
	max_size = len;
}

void Message::init(const uchar *data, int len)
{
	w_data = NULL;
	r_data = data;
	max_size = len;
}

uchar* Message::get_data()
{
	return w_data;
}

const uchar* Message::get_data() const
{
	return r_data;
}

int Message::get_max_size() const
{
	return max_size;
}

bool Message::is_overflowed()
{
	return overflowed;
}


int Message::get_size() const
{
	return cur_size;
}

void Message::set_size(int size)
{
	if (size > max_size)
	{
		cur_size = max_size;
	}
	else
	{
		cur_size = size;
	}
}

int Message::get_write_bit() const
{
	return write_bit;
}

void Message::set_write_bit(int bit)
{
	write_bit = bit & 7;
	if (write_bit) 
	{
		w_data[cur_size-1] &= (1 << write_bit) - 1;
	}
}

int Message::get_num_bits_written() const
{
	return ((cur_size << 3) - ((8 - write_bit) & 7));  
}
  
int Message::get_remaining_write_bits() const
{
	return (max_size << 3) - get_num_bits_written(); 
}

void Message::save_write_state(int& s,int& b) const
{
	s = cur_size;
	b = write_bit;
}

void Message::restore_write_state(int s,int b)
{
	cur_size = s;
	write_bit = b & 7;
	
	if (write_bit) 
	{
		w_data[cur_size-1] &= (1 << write_bit) - 1;
	}  
}

int Message::get_read_count() const
{
	return read_count;
}

void Message::set_read_count(int bytes)
{
	read_count = bytes;
}

int Message::get_read_bit() const
{
	return read_bit;
}

void Message::set_read_bit(int bit)
{
	read_bit = bit & 7;
}

int Message::get_num_bits_read() const
{
	return ((read_count << 3) - ((8 - read_bit) & 7));  
}

int Message::get_remaining_read_bits() const
{
	return (cur_size << 3) - get_num_bits_read();
}

void Message::save_read_state(int& c, int& b) const
{
	c = read_count;
	b = read_bit;
}

void Message::restore_read_state(int c, int b)
{
	read_count = c;
	read_bit = b & 7;
}

void Message::begin_writing()
{
	cur_size = 0;
	write_bit = 0;
	overflowed = false;
}

int Message::get_remaining_space() const
{
	return max_size - cur_size;
}

void Message::write_byte_align()
{
	write_bit = 0;
}

void Message::write_bits(int value, int num_bits)
{
	int		put;
	int		fraction;

	// check if w_data is void
	if (!w_data) 
	{
		os::printf( "cannot write to message" );
	}
	// check if the number of bits is valid
	if (num_bits == 0 || num_bits < -31 || num_bits > 32) 
	{
		os::printf( "bad numBits %i", num_bits);
	}

	// check for value overflows
	// this should be an error really, as it can go unnoticed and cause either bandwidth or corrupted data transmitted
	if (num_bits != 32) 
	{
		if (num_bits > 0) 
		{
			if (value > (1 << num_bits) - 1) 
			{
				os::printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < 0) 
			{
				os::printf( "value overflow %d %d", value, num_bits );
			}
		} 
		else 
		{
			int r = 1 << (-1 - num_bits);
			if (value > r - 1) 
			{
				os::printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < -r) 
			{
				os::printf( "value overflow %d %d", value, num_bits );
			}
		}
	}

	// Change sign if it is negative
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
		if (write_bit == 0) 
		{
			w_data[cur_size] = 0;
			cur_size++;
		}
		
		put = 8 - write_bit;
		if (put > num_bits) 
		{
			put = num_bits;
		}
		
		fraction = value & ((1 << put) - 1);
		w_data[cur_size - 1] |= fraction << write_bit;
		num_bits -= put;
		value >>= put;
		write_bit = (write_bit + put) & 7;
	}
}

void Message::write_char(int c)
{
	write_bits(c, -8);
}

void Message::write_uchar(int c)
{
	write_bits(c, 8);  
}

void Message::write_short(int c)
{
	write_bits(c, -16);  
}

void Message::write_ushort(int c)
{
	write_bits(c, 16);
}

void Message::write_long(int c)
{
	write_bits(c, 32);
}

void Message::write_real(real f)
{
	write_bits(*reinterpret_cast<int *>(&f), 32);  
}

void Message::write_real(real f, int exp_bits, int mant_bits)
{
	//TODO:need to implement floatToBits function
}

void Message::write_angle(real f)
{
	// needs to be implemented
}

void Message::write_vec3(const Vec3& v, int num_bits)
{
	
}

void Message::write_string(const char* s, int max_len, bool make7Bit)
{
  
}

void Message::write_data(const void* data, int length)
{

}

void Message::write_ipv4addr(const os::IPv4Address addr)
{
  
}

void Message::begin_reading() const
{
  
}

int Message::get_remaing_data() const
{
  
}

void Message::read_byte_align() const
{
  
}

int Message::read_bits(int num_bits) const
{
	int		value;
	int		value_bits;
	int		get;
	int		fraction;
	bool	sgn;

	if ( !r_data ) {
		os::printf("cannot read from message");
	}

	// check if the number of bits is valid
	if ( num_bits == 0 || num_bits < -31 || num_bits > 32 ) {
		os::printf("bad number of bits %i", num_bits );
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

	while (value_bits < num_bits ) 
	{
		if (read_bit == 0) 
		{
			read_count++;
		}
		
		get = 8 - read_bit;
		
		if (get > (num_bits - value_bits)) 
		{
			get = num_bits - value_bits;
		}
		
		fraction = r_data[read_count - 1];
		fraction >>= read_bit;
		fraction &= (1 << get) - 1;
		value |= fraction << value_bits;

		value_bits += get;
		read_bit = (read_bit + get) & 7;
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

int Message::read_char() const
{
}

int Message::read_uchar() const
{
}

int Message::read_short() const
{
  
}

int Message::read_ushort() const
{
  
}

int Message::read_long() const
{
  
}

real Message::read_real() const
{
  
}

real Message::read_real(int exp_bits, int mant_bits) const
{
  
}

real Message::read_angle() const
{
  
}

Vec3 Message::read_vec3(int num_bits) const
{
  
}

int Message::read_string(char* buffer, int buffer_size) const
{
  
}

int Message::read_data(void* data, int length) const
{
  
}

void Message::read_ipv4addr(os::IPv4Address* addr) const
{
  
}

// static int		vec3_to_bits(const Vec3& v, int num_bits);
// static Vec3		bits_to_vec3(int bits, int num_bits);
}
}