#pragma once
#include "Types.h"

namespace crown
{
namespace network
{

class PacketQueue
{
public:

	struct PacketData
	{
		uint16_t sequence;	
		uint32_t time;
		size_t size;
	};

public:

							PacketQueue();
							~PacketQueue();

	bool					add(const PacketData& pd);
	bool					get(PacketData& pd);
	size_t					get_total_size() const;
	size_t					get_space_left() const;
	int32_t					get_first() const; 
	int32_t					get_last() const;

private:

	void 					write_uint8(int32_t value);
	void					write_uint16(int32_t value);
	void 					write_int32(int32_t value);

	int32_t					read_uint8();
	int32_t					read_uint16();
	int32_t					read_int32();

private:
	static const uint32_t 	MAX_QUEUE_SIZE = 16384;	

	uint8_t					m_buffer[MAX_QUEUE_SIZE];
	uint32_t				m_first;							// sequence number of first message in queue
	uint32_t				m_last;							// sequence number of last message in queue
	uint32_t				m_start_index;					// index pointing to the first byte of the first message
	uint32_t				m_end_index;						// index pointing to the first byte after the last message
};

} // namespace network
} // namespace crown
