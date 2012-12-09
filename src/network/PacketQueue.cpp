#include "PacketQueue.h"

namespace crown
{
namespace network
{

//-------------------------------------------------------------
PacketQueue::PacketQueue() :
	m_first(0),
	m_last(0),
	m_start_index(0),
	m_end_index(0)
{

}

//-------------------------------------------------------------
PacketQueue::~PacketQueue()
{

}

//-------------------------------------------------------------
bool PacketQueue::add(const PacketData& pd)
{	
	// update last sequence number
	m_last = pd.sequence; 
	// update end index
	m_end_index += sizeof(PacketData);
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
void PacketQueue::write_uint8(uint8_t value)
{
	m_buffer[m_end_index] = value;
	m_end_index = (m_end_index + 1) & (MAX_QUEUE_SIZE - 1);
}

//-------------------------------------------------------------
void PacketQueue::write_uint16(int32_t value)
{
	write_uint8((value >> 0) & 255); 
	write_uint8((value >> 8) & 255);
}

//-------------------------------------------------------------
void PacketQueue::write_int32(int32_t value)
{
	write_uint8((value >> 0) & 255); 
	write_uint8((value >> 8) & 255);
	write_uint8((value >> 16) & 255); 
	write_uint8((value >> 24) & 255);	
}

//-------------------------------------------------------------
int8_t PacketQueue::read_uint8()
{
	uint8_t value = m_buffer[m_start_index];
	m_start_index = (m_start_index + 1) & (MAX_QUEUE_SIZE - 1);
}

//-------------------------------------------------------------
int32_t PacketQueue::read_uint16()
{
	return (read_uint8()) | (read_uint8() << 8);
}

//-------------------------------------------------------------
int32_t PacketQueue::read_int32()
{
	return (read_uint8()) | (read_uint8() << 8) | (read_uint8() << 16) | (read_uint8() << 24);
}

}
} // namespace crown
