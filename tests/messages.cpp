#include <cstdio>

#include "Vec3.h"
#include "OS.h"
#include "BitMessage.h"
#include "MallocAllocator.h"

using namespace crown;

void test_int8()
{
	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 
	
	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);
	
	int8_t res;
	
	m.init_in_w(4);
	m.begin_writing();
	m.write_int8(-56);
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

}

void test_uint8()
{
	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 
	
	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);

	uint8_t res;
	
	m.init_in_w(4);
	m.begin_writing();
	m.write_uint8(255);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();

	res = m.read_uint8();
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
}

void test_int16()
{
	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 
	
	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);  
	
	int16_t res;
	
	m.init_in_w(4);
	m.write_int16(-5555);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();

	res = m.read_int16();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for INT16\n");
	printf("value = %d\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");
}

void test_uint16()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);

	uint16_t res;
	
	m.init_in_w(4);
	m.write_uint16(5555);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();

	res = m.read_uint16();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for UINT16\n");
	printf("value = %d\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");
}

void test_int32()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);
	
	int32_t res;
	
	m.init_in_w(4);
	m.write_int32(4000000);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();
	
	res = m.read_int32();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for INT32\n");
	printf("value = %d\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");
	
}

void test_real()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);

	real res;
	
	m.init_in_w(4);
	m.write_real(4.5342f);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();
	
	res = m.read_real();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for REAL\n");
	printf("value = %f\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");	
	

}

void test_vec3()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);
	
	
	Vec3 v(0.525f, 0.432f, 0.234f);
	Vec3 res;
	
	m.init_in_w(12);
	m.write_vec3(v);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();
	
	res = m.read_vec3();
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for VEC_3\n");
	printf("x = %f, y = %f, z = %f\n", res.x, res.y, res.z);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");	
}

void test_string()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);
	
	uint8_t tmp[16];
	char res[16];

	char s[] = "test";
	
	m.init_in_w(16);

	m.write_string(s, sizeof(s), true);
 	bits_written = m.get_num_bits_written();
  	rem_write_bits = m.get_remaining_write_bits();	
	
	m.read_string(res, 6);
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();

	printf("-----------------------------\n");
	printf("start write and read for STRING\n");
 	printf("string = %s\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");	

}

void test_data()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 

	MallocAllocator allocator;
  	network::BitMessage m = network::BitMessage(allocator);
	
	uint8_t tmp[] = "test generic";
	uint8_t res[16];
	
	m.init_in_w(16);
	
	m.write_data(tmp, 16);
 	bits_written = m.get_num_bits_written();
  	rem_write_bits = m.get_remaining_write_bits();	
	
	m.read_data(res, 16);
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for GENERIC\n");
	printf("string = %s\n", res);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");	

}

void test_net_address()
{
  	uint32_t bits_written;
	uint32_t rem_write_bits;
	uint32_t bits_read;
	uint32_t rem_read_bits; 
	
	MallocAllocator allocator;
	network::BitMessage m = network::BitMessage(allocator);

	uint8_t tmp[16];
	
	
	os::NetAddress addr;
	os::NetAddress res;
	
	addr.set(192, 168, 0, 1, 80);
	
	m.init_in_w(16);
	m.write_ipv4addr(addr);
	bits_written = m.get_num_bits_written();
	rem_write_bits = m.get_remaining_write_bits();	
	
	m.read_ipv4addr(&res);
	bits_read = m.get_num_bits_read();
	rem_read_bits = m.get_remaining_read_bits();
	
	printf("-----------------------------\n");
	printf("start write and read for NET_ADDRESS\n");
	printf("a = %d, b = %d, c = %d, d = %d, p = %d\n", res.address[0], res.address[1], res.address[2], res.address[3], res.port);
	printf("bits written = %d\n", bits_written);
	printf("remaining write bits = %d\n", rem_write_bits);
	printf("bits read = %d\n", bits_read);
	printf("remaining read bits = %d\n", rem_read_bits);
	printf("-----------------------------\n");
	printf("\n");	
}

int main()
{
	
	test_int8();
	test_uint8();
	test_int16();
	test_uint16();
	test_int32();
	test_real();
	test_vec3();
	test_string();
	test_data();
	test_net_address();
	
	return 0;
}