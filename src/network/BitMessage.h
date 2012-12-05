#pragma once

#include "Types.h"
#include "OS.h"
#include "Allocator.h"
#include "Vec3.h"

namespace crown
{
namespace network
{
	/**
	* bit-packet reliable message.
	* Usage: After every instantition, must be initialized with @init(len)
	*/

class BitMessage
{

public:
						BitMessage(Allocator& allocator);
						~BitMessage();

	void				init(int32_t len);									// init with data length in byte
	void				set_header(uint32_t id, uint16_t sequence, uint16_t ack, uint32_t ack_bits);
	uint8_t*			get_header();										// get message header for writing
	const uint8_t*		get_header() const;									// get message header for reading
	uint8_t*			get_data();											// get message data for writing
	const uint8_t*		get_data() const;									// get message data for reading
	size_t				get_max_size() const;								// get the maximum message size
	bool 				is_overflowed();									// is message overflowed
	bool				is_init();											// is message initialized
	
	size_t				get_header_size() const;							// return 12 bytes
	size_t				get_size() const;									// size of the message in bytes
	void				set_size(size_t size);								// set the message size
	int32_t				get_write_bit() const;								// get current write bit
	void				set_write_bit(int32_t bit);							// set current write bit
	int32_t				get_num_bits_written() const;						// returns number of bits written
	int32_t				get_remaining_write_bits() const;					// space left in bits for writing
	void				save_write_state(int32_t& s,int32_t& b) const;		// save the write state
	void				restore_write_state(int32_t s,int32_t b);			// restore the write state

	int32_t				get_read_count() const;								// bytes read so far
	void				set_read_count(int32_t bytes);						// set the number of bytes and bits read
	int32_t				get_read_bit() const;								// get current read bit
	void				set_read_bit(int32_t bit);							// set current read bit
	int32_t				get_num_bits_read() const;							// returns number of bits read
	int32_t				get_remaining_read_bits() const;					// number of bits left to read
	void				save_read_state(int32_t& c, int32_t& b) const;		// save the read state
	void				restore_read_state(int32_t c, int32_t b);			// restore the read state
						// write state utilities
	void				begin_writing();									// begin writing
	int32_t				get_remaining_space() const;						// space left in bytes
	void				write_byte_align();									// write up to the next byte boundary
	void				write_bits(int32_t value, int32_t num_bits);		// write the specified number of bits
	void				write_int8(int32_t c);
	void				write_uint8(int32_t c);
	void				write_int16(int32_t c);
	void				write_uint16(int32_t c);
	void				write_int32(int32_t c);
	void				write_real(real f);
	void				write_vec3(const Vec3& v);
	void				write_string(const char* s, int32_t max_len = -1, bool make_7_bit = true);
	void				write_data(const void* data, int32_t length);
	void				write_netaddr(const os::NetAddress addr);

						// read state utilities
	void				begin_reading() const;								// begin reading.
	int32_t				get_remaing_data() const;							// number of bytes left to read
	void				read_byte_align() const;							// read up to the next byte boundary
	int32_t				read_bits(int32_t num_bits) const;					// read the specified number of bits
	int32_t				read_int8() const;
	int32_t				read_uint8() const;
	int32_t				read_int16() const;
	int32_t				read_uint16() const;
	int32_t				read_int32() const;
	real				read_real() const;
	Vec3				read_vec3() const;
	int32_t				read_string(char* buffer, int32_t buffer_size) const;
	int32_t				read_data(void* data, int32_t length) const;
	void				read_netaddr(os::NetAddress* addr) const;
	
	void				print() const;

private:
  
	uint8_t*			get_byte_space(int32_t len);
	bool 				check_overflow(int32_t num_bits);			// check buffer overflow	
	
private:
	
	Allocator*			m_allocator;								// memory allocator
	
	uint8_t*			m_header;
	uint8_t*			m_data;
	uint8_t*			m_write;									// pointer to data for writing
	const uint8_t*		m_read;										// point32_ter to data for reading
	
	size_t				m_max_size;									// maximum size of message in bytes
	size_t				m_cur_size;									// current size of message in bytes
	mutable int32_t		m_read_count;								// number of bytes read so far
	
	int32_t				m_write_bit;								// number of bits written to the last written byte
	mutable int32_t		m_read_bit;									// number of bits read from the last read byte
	
	bool 				m_overflowed;								// overflow flag
	bool				m_init;										// is init flag
	
};
} // namespace network
} // namespace crown

