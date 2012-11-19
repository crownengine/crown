#pragma once

#include "Types.h"
#include "OS.h"
#include "Vec3.h"

namespace crown
{
namespace network
{
	/**
	* bit-packet reliable message
	*/
	
	class Message
	{
	public:
						Message();
						~Message();

		void			init(uint8_t *data, int32_t len);
		void			init(const uint8_t *data, int32_t len);
		uint8_t*		get_data();										// get data for writing
		const uint8_t*	get_data() const;								// get data for reading
		int32_t			get_max_size() const;							// get the maximum message size
		bool 			is_overflowed();								// get overflowed flag


		int32_t			get_size() const;								// size of the message in bytes
		void			set_size(int32_t size);							// set the message size
		int32_t			get_write_bit() const;							// get current write bit
		void			set_write_bit(int32_t bit);						// set current write bit
		int32_t			get_num_bits_written() const;					// returns number of bits written
		int32_t			get_remaining_write_bits() const;				// space left in bits for writing
		void			save_write_state(int32_t& s,int32_t& b) const;	// save the write state
		void			restore_write_state(int32_t s,int32_t b);		// restore the write state

		int32_t			get_read_count() const;							// bytes read so far
		void			set_read_count(int32_t bytes);					// set the number of bytes and bits read
		int32_t			get_read_bit() const;							// get current read bit
		void			set_read_bit(int32_t bit);						// set current read bit
		int32_t			get_num_bits_read() const;						// returns number of bits read
		int32_t			get_remaining_read_bits() const;				// number of bits left to read
		void			save_read_state(int32_t& c, int32_t& b) const;	// save the read state
		void			restore_read_state(int32_t c, int32_t b);		// restore the read state

		void			begin_writing();								// begin writing
		int32_t			get_remaining_space() const;					// space left in bytes
		void			write_byte_align();								// write up to the next byte boundary
		void			write_bits(int32_t value, int32_t num_bits);	// write the specified number of bits
		void			write_int8(int32_t c);
		void			write_uint8(int32_t c);
		void			write_int16(int32_t c);
		void			write_uint16(int32_t c);
		void			write_int32(int32_t c);
		void			write_real(real f);
		void			write_vec3(const Vec3& v, int32_t num_bits);
		void			write_string(const char* s, int32_t max_len = -1, bool make_7_bit = true);
		void			write_data(const void* data, int32_t length);
		void			write_ipv4addr(const os::IPv4Address addr);

		void			begin_reading() const;					// begin reading.
		int32_t			get_remaing_data() const;				// number of bytes left to read
		void			read_byte_align() const;				// read up to the next byte boundary
		int32_t			read_bits(int32_t num_bits) const;			// read the specified number of bits
		int32_t			read_int8() const;
		int32_t			read_uint8() const;
		int32_t			read_int16() const;
		int32_t			read_uint16() const;
		int32_t			read_int32() const;
		real			read_real() const;
		Vec3			read_vec3(int32_t num_bits) const;
		int32_t			read_string(char* buffer, int32_t buffer_size) const;
		int32_t			read_data(void* data, int32_t length) const;
		void			read_ipv4addr(os::IPv4Address* addr) const;

 		static int32_t	vec3_to_bits(const Vec3& v, int32_t num_bits);
 		static Vec3		bits_to_vec3(int32_t bits, int32_t num_bits);

	private:
	  
		uint8_t*			w_data;			// pointer to data for writing
		const uint8_t*		r_data;			// point32_ter to data for reading
		int32_t				max_size;		// maximum size of message in bytes
		int32_t				cur_size;		// current size of message in bytes
		int32_t				write_bit;		// number of bits written to the last written byte
		mutable int32_t		read_count;		// number of bytes read so far
		mutable int32_t		read_bit;		// number of bits read from the last read byte
		bool 				overflowed;		// overflow flag

	private:
	  
		uint8_t*		get_byte_space(int32_t len);
		bool 			check_overflow(int32_t num_bits);			// check buffer overflow	
  };
} // namespace network
} // namespace crown

