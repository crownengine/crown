#include <cstdio>

#include "Message.h"

using namespace crown;

int main()
{

	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits;

//------------------------------------------------------------------
  	network::Message m = network::Message();

	uint8_t tmp[4];
	uint8_t res;
	
	m.init(tmp, 4);
	m.begin_writing();
	m.write_uint8(255);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();

	res = m.read_int8();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("\n-----------------------------\n");
	printf("start write and read for UINT8\n");
	printf("value = %d\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");

	printf("\n");
	
//------------------------------------------------------------------	
	
  	network::Message m1 = network::Message();
	
	uint8_t tmp1[4];
	int8_t res1;

	m1.init(tmp1, 4);
	m1.write_int8(-56);
	bits_written = m1.get_num_bits_written();
	rem_write_bits = m1.get_remaining_write_bits();

	res1 = m1.read_int8();
	bits_read = m1.get_num_bits_read();
	rem_read_bits = m1.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for INT8\n");
	printf("value = %d\n", res1);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");

	printf("\n");
	
//------------------------------------------------------------------	

	network::Message m2 = network::Message();
	
	uint8_t tmp2[4];
	int16_t res2;
	
	m2.init(tmp2, 4);
	m2.write_int16(5555);
	bits_written = m2.get_num_bits_written();
	rem_write_bits = m2.get_remaining_write_bits();

	res1 = m2.read_int16();
	bits_read = m2.get_num_bits_read();
	rem_read_bits = m2.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for INT16\n");
	printf("value = %d\n", res2);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");

	printf("\n");
	

	
	
	return 0;
}