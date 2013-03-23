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
	if (get_space_left() < sizeof(PacketData))
	{
		return false;
	}
	// update last sequence number
	write_uint16(pd.sequence);
	write_int32(pd.time);
	write_int32(pd.time);
	last++; 
	return true;
}

//-------------------------------------------------------------
bool PacketQueue::get(PacketData& pd)
{	
	// empty queue
	if (first == last)
	{
		return false;
	}

	pd.sequence = read_uint16();
	pd.time = read_int32();
	pd.size = read_int32();

	assert(pd.sequence == first);
	first++;
	return true;
}

//-------------------------------------------------------------
size_t PacketQueue::get_total_size() const
{
	if (m_start_index <= m_end_index) 
	{
		return endIndex - startIndex;
	} 
	else 
	{
		return sizeof(buffer) - startIndex + endIndex;
	}
}

//-------------------------------------------------------------
size_t PacketQueue::get_space_left() const
{
	if (m_start_index <= m_end_index) 
	{
		return sizeof(buffer) - (endIndex - startIndex) - 1;
	} 
	else 
	{
		return (startIndex - endIndex) - 1;
	}
}

//-------------------------------------------------------------
int32_t PacketQueue::get_first() const
{
	return m_first;
}

//-------------------------------------------------------------
int32_t PacketQueue::get_last() const
{
	return m_last;
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
