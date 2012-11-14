#pragma once

#include "IPv4Address.h"

namespace crown
{
  
namespace network
{
	typedef unsigned char byte;
	
	class Vec3;
	class Angle;
	
	class Message
	{
						Message();
						~Message();

		void			init(byte* data, int length);
		void			init(const byte* data, int length);
		byte*			get_data();								// get data for writing
		const byte*		get_data() const;						// get data for reading
		int				get_max_size() const;					// get the maximum message size

		int				get_size() const;						// size of the message in bytes
		void			set_size(int size);						// set the message size
		int				get_write_bit() const;					// get current write bit
		void			set_write_bit(int bit);					// set current write bit
		int				get_num_bits_written() const;			// returns number of bits written
		int				get_remaining_write_bits() const;		// space left in bits for writing
		void			save_write_state(int& s, int& b) const;	// save the write state
		void			restore_write_state(int s, int b);		// restore the write state

		int				get_read_count() const;					// bytes read so far
		void			set_read_count(int bytes);				// set the number of bytes and bits read
		int				get_read_bit() const;					// get current read bit
		void			set_read_bit(int bit);					// set current read bit
		int				get_num_bits_read() const;				// returns number of bits read
		int				get_remaining_read_bits() const;		// number of bits left to read
		void			save_read_state(int& c, int& b) const;	// save the read state
		void			restore_read_state(int c, int b);		// restore the read state

		void			begin_writing();						// begin writing
		int				get_remaining_space() const;			// space left in bytes
		void			write_byte_align();						// write up to the next byte boundary
		void			write_bits(int value, int num_bits);	// write the specified number of bits
		void			write_char(int c);
		void			write_byte(int c);
		void			write_short(int c);
		void			write_u_short(int c);
		void			write_long(int c);
		void			write_float(float f);
		void			write_float(float f, int exponent_bits, int mantissa_bits);
		void			write_angle(Angle angle);
		void			write_vec3(const Vec3& dir, int num_bits);
		void			write_string(const char* s, int max_length = -1, bool make7Bit = true );
		void			write_data(const void* data, int length);
		void			write_ipv4addr(const IPv4Address addr);

		void			begin_reading() const;					// begin reading.
		int				get_remaing_data() const;				// number of bytes left to read
		void			read_byte_align() const;				// read up to the next byte boundary
		int				read_bits(int num_bits) const;			// read the specified number of bits
		int				read_char() const;
		int				read_byte() const;
		int				read_short() const;
		int				read_u_short() const;
		int				read_long() const;
		float			read_float() const;
		float			read_float(int exponent_bits, int mantissa_bits) const;
		Angle			read_angle() const;
		Vec3			read_vec3(int num_bits) const;
		int				read_string(char* buffer, int buffer_size) const;
		int				read_data(void* data, int length) const;
		void			read_ipv4addr(IPv4Address* addr) const;

		static int		vec3_to_bits(const Vec3& vec, int num_bits);
		static Vec3		bits_to_vec3(int bits, int numBits);

	private:
		byte *			m_write_data;			// pointer to data for writing
		const byte *	m_read_data;			// pointer to data for reading
		int				m_max_size;				// maximum size of message in bytes
		int				m_cur_size;				// current size of message in bytes
		int				m_write_bit;			// number of bits written to the last written byte
		mutable int		m_read_count;			// number of bytes read so far
		mutable int		m_read_bit;				// number of bits read from the last read byte

	private:
		byte *			_get_byte_space(int length);
	};
}
}